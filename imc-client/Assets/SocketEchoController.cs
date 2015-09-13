using UnityEngine;
using System;
using System.Collections;
using System.IO;
using System.Net;
using System.Net.Sockets;
using System.Text;

public class SocketEchoController : MonoBehaviour {
	private SocketClient socketClient;

	private float lastUpdateTime = 0;
	private float updateTimeInterval = 0.001F;

	private Vector3 position;
	private Quaternion rotation;

	void Start() {
		socketClient = new SocketClient("192.168.1.9", 1234, socketReadCallback);

		socketClient.connect ();
	}

	void Update() {
		if (Math.Abs(Time.time - lastUpdateTime) >= updateTimeInterval) {
			lastUpdateTime = Time.time;

			socketClient.write ("NEXT");
			socketClient.read ();

			transform.position = position;
			transform.rotation  = rotation;
		}
	}

	public void exitButtonClicked() {
		socketClient.write ("EXIT");
		socketClient.disconnect ();
		Application.Quit();
		UnityEditor.EditorApplication.isPlaying = false;
	}

	void socketReadCallback(string readResult) {
		string[] stringParts = readResult.Split (" "[0]);
		float[] parts = new float[stringParts.Length];

		for (int i = 0; i < stringParts.Length; i++) {
			parts[i] = float.Parse(stringParts[i]);
		}

		if (parts.Length != 7) {
			Debug.LogError ("Malformed server response \"" + readResult + "\"");
		} else {
			position.x = parts[0];
			position.y = parts[1];
			position.z = parts[2];

			rotation.w = parts[3];
			rotation.x = parts[4];
			rotation.y = parts[5];
			rotation.z = parts[6];

			Debug.Log ("position => " + position + " rotation => " + rotation);
		}
	}
}
