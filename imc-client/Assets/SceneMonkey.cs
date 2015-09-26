using UnityEngine;
using UnityEngine.UI;
using System;

public class SceneMonkey : MonoBehaviour
{
    // Config
    private const bool LOG_ROTATION = false;
    private const bool SOCKET_ACTIVE = true;

    public String edison_ip;

    // Socket
    private SocketClient socket_client;

    private float last_update_time = 0;
    private float update_time_interval = 0.0001F;

    // Catapult parts
    public GameObject monkey;

    // Transform
    Vector3 calibration_rotation = new Vector3(90, 0, 180);
    Vector3 offset_rotation;
    Vector3 server_rotation;

    // Misc
    private bool initial_offsets_calculated = false;

    /* Unity Lifecyle */
    void Start()
    {
        socket_client = new SocketClient(edison_ip, 1234, socket_read_callback);

        if (SOCKET_ACTIVE)
        {
            socket_client.connect();
        }
    }

    void Update()
    {
        // Update rotation
        if (Math.Abs(Time.time - last_update_time) >= update_time_interval)
        {
            last_update_time = Time.time;

            // Apply rotation
            // X => Pitch
            // Y => Heading
            // Z => Roll
            monkey.transform.rotation = Quaternion.Euler(normalize_angle(server_rotation - offset_rotation - calibration_rotation));

            // Request new rotation
            if (SOCKET_ACTIVE)
            {
                socket_client.write("NEXT");
                socket_client.read();
            }

            if (LOG_ROTATION)
            {
                Debug.Log("server => " + server_rotation);
            }
        }
    }

    /* Helpers */
    public void complete_exit()
    {
        if (SOCKET_ACTIVE)
        {
            socket_client.disconnect();
        }
        Application.Quit();
        UnityEditor.EditorApplication.isPlaying = false;
    }


    public float normalize_angle(float angle)
    {
        while (angle > 360)
        {
            angle -= 360;
        }

        while (angle < 0)
        {
            angle += 360;
        }

        return angle;
    }

    public Vector3 normalize_angle(Vector3 angles)
    {
        return new Vector3(
            normalize_angle(angles.x),
            normalize_angle(angles.y),
            normalize_angle(angles.z)
         );
    }
    
    public void calculate_offsets()
    {
        offset_rotation = server_rotation;
    }

    /* UI Callbacks */
    public void on_kill_button_clicked()
    {
        socket_client.write("KILL");
        complete_exit();
    }

    public void on_disconnect_button_clicked()
    {
        socket_client.write("EXIT");
        complete_exit();
    }

    public void on_reset_button_clicked()
    {
        calculate_offsets();
    }

    public void on_target_reset_button_click()
    {
        socket_client.write("EXIT");
        Application.LoadLevel("Demo-Monkey");
    }

    public void on_go_to_game_start_button_clicked()
    {
        socket_client.write("EXIT");
        Application.LoadLevel("Demo-Start");
    }

    /* Socket Callbacks */
    void socket_read_callback(string read_result)
    {
        string[] string_parts = read_result.Split(" "[0]);
        float[] parts = new float[string_parts.Length];

        for (int i = 0; i < string_parts.Length; i++)
        {
            parts[i] = float.Parse(string_parts[i]);
        }

        if (parts.Length != 3)
        {
            Debug.LogError("Malformed server response \"" + read_result + "\"");
        }
        else
        {
            server_rotation.x = parts[0];
            server_rotation.y = parts[1];
            server_rotation.z = parts[2];

            server_rotation = normalize_angle(server_rotation);
            
            if (!initial_offsets_calculated)
            {
                calculate_offsets();
                initial_offsets_calculated = true;
            }
        }
    }
}
