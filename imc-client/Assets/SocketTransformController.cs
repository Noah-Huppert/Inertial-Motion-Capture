using UnityEngine;
using System;

public class SocketTransformController : MonoBehaviour {
    // Config
    private const bool LOG_ROTATION = false;
    private const bool SOCKET_ACTIVE = true;

    // Socket
    private SocketClient socket_client;

    private float last_update_time = 0;
    private float update_time_interval = 0.0001F;

    // Catapult parts
    public GameObject catapult_base;
    public GameObject catapult_arm;

    // Transform
    Vector3 calibration_rotation = new Vector3(270, 0, 0);
    Vector3 offset_rotation;
    Vector3 server_rotation;

    // Game stages
    enum GameStage {
        AIM,
        POWER,
        FIRE
    }

    float stage_aim_value = 0;
    GameStage stage = GameStage.AIM;

    // Misc
    private bool initial_offsets_calculated = false;

    /* Unity Lifecyle */
    void Start() {
        socket_client = new SocketClient("192.168.1.9", 1234, socket_read_callback);

        if(SOCKET_ACTIVE) {
            socket_client.connect();
        }
    }

    void Update() {
        // Update rotation
        if(Math.Abs(Time.time - last_update_time) >= update_time_interval) {
            last_update_time = Time.time;

            // Apply rotation
            // X => Pitch
            // Y => Heading
            // Z => Roll
            Vector3 allowed_rotation = new Vector3();

            if(stage == GameStage.AIM) {
                allowed_rotation.y = server_rotation.y;
            } else if(stage == GameStage.POWER) {
                allowed_rotation.x = server_rotation.x;
                allowed_rotation.y = stage_aim_value;
            } else if(stage == GameStage.FIRE) {
                allowed_rotation.y = stage_aim_value;
            }

            float limit_difference = 75 - allowed_rotation.x;
            if(limit_difference < 0 && limit_difference >= -195) {
                allowed_rotation.x = 75;
            } else if(limit_difference >= -285 && limit_difference < -195) {
                allowed_rotation.x = 0;
            }

            Debug.Log("limit_difference => " + limit_difference);
             
            Vector3 calculated_rotation = normalize_angle(allowed_rotation - offset_rotation - calibration_rotation);

            if(stage != GameStage.FIRE) {
                catapult_arm.transform.localRotation = Quaternion.AngleAxis(calculated_rotation.x * -1, Vector3.right);
            }

            catapult_base.transform.localRotation = Quaternion.AngleAxis(calculated_rotation.y, Vector3.up);

            // Request new rotation
            if(SOCKET_ACTIVE) {
                socket_client.write("NEXT");
                socket_client.read();
            }
            
            /*
            if(stage == GameStage.POWER || stage == GameStage.FIRE) {
                server_rotation.y = stage_aim_value;
            }

            Vector3 calculated_rotation = normalize_angle(server_rotation - offset_rotation - calibration_rotation);

            if(stage == GameStage.AIM) {
                catapult_base.transform.localRotation = Quaternion.AngleAxis(calculated_rotation.y, Vector3.up);
            }

            if(stage == GameStage.POWER) {
                catapult_arm.transform.localRotation = Quaternion.AngleAxis(calculated_rotation.x * -1, Vector3.right);
            }
            */

            if(LOG_ROTATION) {
                Debug.Log("server => " + server_rotation);
            }
        }

        // Keypresses
        if(Input.GetKeyUp(KeyCode.Space)) {
            if(stage == GameStage.AIM) {// Aim => Power
                stage = GameStage.POWER;
                stage_aim_value = server_rotation.y;
            } else if(stage == GameStage.POWER) {// Power => Fire
                stage = GameStage.FIRE;
            } else if(stage == GameStage.FIRE) {// Fire => Aim
                stage = GameStage.AIM;
            }
        }
    }

    /* Helpers */
    public void complete_exit() {
        if(SOCKET_ACTIVE) {
            socket_client.disconnect();
        }
        Application.Quit();
        UnityEditor.EditorApplication.isPlaying = false;
    }

    public float normalize_angle(float angle) {
        while(angle > 360) {
            angle -= 360;
        }

        while(angle < 0) {
            angle += 360;
        }

        return angle;
    }

    public Vector3 normalize_angle(Vector3 angles) {
        return new Vector3(
            normalize_angle(angles.x),
            normalize_angle(angles.y),
            normalize_angle(angles.z)
         );
    }

    public void calculate_offsets() {
        offset_rotation = server_rotation;
    }

    /* UI Callbacks */
    public void on_kill_button_clicked() {
        socket_client.write("KILL");
        complete_exit();
    }

    public void on_disconnect_button_clicked() {
        socket_client.write("EXIT");
        complete_exit();
    }

    public void on_reset_button_clicked() {
        calculate_offsets();
    }

    /* Socket Callbacks */
    void socket_read_callback(string read_result) {
        string[] string_parts = read_result.Split(" "[0]);
        float[] parts = new float[string_parts.Length];

        for(int i = 0; i < string_parts.Length; i++) {
            parts[i] = float.Parse(string_parts[i]);
        }

        if(parts.Length != 3) {
            Debug.LogError("Malformed server response \"" + read_result + "\"");
        } else {
            server_rotation.x = parts[0];
            server_rotation.y = parts[1];
            server_rotation.z = parts[2];

            server_rotation = normalize_angle(server_rotation);

            if(!initial_offsets_calculated) {
                calculate_offsets();
                initial_offsets_calculated = true;
            }
        }
    }
}
