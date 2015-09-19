using UnityEngine;
using System;

public class SocketTransformController : MonoBehaviour {
    private const bool LOG_POSITION = true;
    private const bool SOCKET_ACTIVE = true;

    private SocketClient socketClient;

    private float lastUpdateTime = 0;
    private float updateTimeInterval = 0.0001F;

    private const long position_dampening_constant = 150;

    private Vector3 position;
    private Quaternion rotation;

    /* Unity Lifecyle */
    void Start() {
        socketClient = new SocketClient("192.168.1.9", 1234, socketReadCallback);

        if(SOCKET_ACTIVE) {
            socketClient.connect();
        }
    }

    void Update() {
        if(Math.Abs(Time.time - lastUpdateTime) >= updateTimeInterval) {
            lastUpdateTime = Time.time;

            if(SOCKET_ACTIVE) {
                socketClient.write("NEXT");
                socketClient.read();
            }

            position.x /= position_dampening_constant;
            position.y /= position_dampening_constant;
            position.z /= position_dampening_constant;

            transform.localPosition = position;
            transform.localRotation = rotation;

            if(LOG_POSITION) {
                Debug.Log("position => " + position + " rotation => " + rotation);
            }
        }
    }

    /* Helpers */
    public void completeExit() {
        if(SOCKET_ACTIVE) {
            socketClient.disconnect();
        }
        Application.Quit();
        UnityEditor.EditorApplication.isPlaying = false;
    }

    /* UI Callbacks */
    public void onKillButtonClicked() {
        Debug.Log("Kill button clicked");
        socketClient.write("KILL");
        completeExit();
    }

    public void onDisconnectButtonClicked() {
        Debug.Log("Disconnect button clicked");
        socketClient.write("EXIT");
        completeExit();
    }

    public void onResetButtonClicked() {
        Debug.Log("Reset button clicked");
        socketClient.write("RESET");
    }

    /* Socket Callbacks */
    void socketReadCallback(string readResult) {
        string[] stringParts = readResult.Split(" "[0]);
        float[] parts = new float[stringParts.Length];

        for(int i = 0; i < stringParts.Length; i++) {
            parts[i] = float.Parse(stringParts[i]);
        }

        if(parts.Length != 7) {
            Debug.LogError("Malformed server response \"" + readResult + "\"");
        } else {
            //position.x = parts[2];
            //position.y = parts[0];
            //position.z = parts[1];

            rotation.w = parts[3];
            rotation.x = parts[4];
            rotation.y = parts[5];
            rotation.z = parts[6];
        }
    }
}
