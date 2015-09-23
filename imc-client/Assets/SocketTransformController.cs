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
    public GameObject catapult_basket;
    public GameObject ball_prefab;

    // Transform
    Vector3 calibration_rotation = new Vector3(330, 0, 0);
    Vector3 offset_rotation;
    Vector3 server_rotation;

    // Game stages
    enum GameStage {
        AIM,
        POWER,
        FIRE
    }
    
    GameStage stage = GameStage.AIM;
    float stage_aim_value = 0;
    float stage_fire_start_time = -1;
    bool stage_fire_ball_spawned = false;

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

            // Lock axies based on stage
            if(stage == GameStage.AIM) {
                allowed_rotation.x = offset_rotation.x;
                allowed_rotation.y = server_rotation.y;
            } else if(stage == GameStage.POWER) {
                allowed_rotation.x = server_rotation.x;
                allowed_rotation.y = stage_aim_value;
            } else if(stage == GameStage.FIRE) {
                allowed_rotation.y = stage_aim_value;
            }

            // Limit arm rotation
            float offset_server_x = normalize_angle(server_rotation.x - offset_rotation.x);
            
            if(offset_server_x <= 360 && offset_server_x >= 150) {
                allowed_rotation.x = offset_rotation.x;
            } else if(offset_server_x >= 95 && offset_server_x < 150) {
                allowed_rotation.x = offset_rotation.x + 95;
            }

            // Calculate final rotation
            Vector3 calculated_rotation = normalize_angle(allowed_rotation - offset_rotation - calibration_rotation);

            // Set arm rotation
            if(stage != GameStage.FIRE) {// AIM || POWER
                catapult_arm.transform.localRotation = Quaternion.AngleAxis(calculated_rotation.x * -1, Vector3.right);
            } else if(catapult_arm.transform.rotation.eulerAngles.x <= calibration_rotation.x) {// FIRE in progress
                if(stage_fire_start_time == -1) {// Just fired
                    stage_fire_start_time = Time.time;
                    stage_fire_ball_spawned = false;
                }

                catapult_arm.transform.Rotate(5, 0, 0);
            }  else {// FIRE complete
                if(!stage_fire_ball_spawned) {
                    float fire_time = (Time.time - stage_fire_start_time) * 50;

                    GameObject ball = Instantiate(ball_prefab);
                    ball.transform.position = catapult_basket.transform.position;
                    ball.transform.rotation = Quaternion.AngleAxis(calibration_rotation.x, Vector3.right);

                    ball.GetComponent<Rigidbody>().AddForce(0, 0, 200 * fire_time);

                    stage_fire_ball_spawned = true;
                    stage_fire_start_time = -1;
                }
            }

            // Set base rotation
            catapult_base.transform.localRotation = Quaternion.AngleAxis(calculated_rotation.y, Vector3.up);

            // Request new rotation
            if(SOCKET_ACTIVE) {
                socket_client.write("NEXT");
                socket_client.read();
            }

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
