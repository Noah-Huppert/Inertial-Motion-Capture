using UnityEngine;
using System;
using System.Collections;
using System.Collections.Generic;
using System.IO;
using System.Net;
using System.Net.Sockets;
using System.Text;

public class SocketEchoController : MonoBehaviour {
	private SocketClient socketClient;

	private float lastUpdateTime = 0;
	private float updateTimeInterval = 0.0001F;

	private const int position_maf_size = 1;
	private const long position_dampening_constant = 100000000;//5000000000;

	private MovingAverageFilter position_x_maf = new MovingAverageFilter (position_maf_size);
	private MovingAverageFilter position_y_maf = new MovingAverageFilter (position_maf_size);
	private MovingAverageFilter position_z_maf = new MovingAverageFilter (position_maf_size);

	private Quaternion rotation;

	/* Unity Lifecyle */
	void Start() {
		socketClient = new SocketClient("192.168.1.9", 1234, socketReadCallback);

		socketClient.connect ();
	}

	void Update() {
		if (Math.Abs(Time.time - lastUpdateTime) >= updateTimeInterval) {
			lastUpdateTime = Time.time;

			socketClient.write ("NEXT");
			socketClient.read ();

			Vector3 position = new Vector3(
				position_x_maf.average() / position_dampening_constant,
				position_y_maf.average() / position_dampening_constant,
				position_z_maf.average() / position_dampening_constant
			);

			transform.localPosition = position;
			transform.localRotation = rotation;

			Debug.Log ("position => " + position + " rotation => " + rotation);
		}
	}

	/* Helpers */
	public void completeExit() {
		socketClient.disconnect ();
		Application.Quit();
		UnityEditor.EditorApplication.isPlaying = false;
	}

	/* UI Callbacks */
	public void onKillButtonClicked() {
		socketClient.write ("KILL");
		completeExit ();
	}

	public void onDisconnectButtonClicked() {
		socketClient.write ("EXIT");
		completeExit ();
	}

	public void onResetButtonClicked() {
		socketClient.write ("RESET");
	}

	/* Socket Callbacks */
	void socketReadCallback(string readResult) {
		string[] stringParts = readResult.Split (" "[0]);
		float[] parts = new float[stringParts.Length];

		for (int i = 0; i < stringParts.Length; i++) {
			parts[i] = float.Parse(stringParts[i]);
		}

		if (parts.Length != 7) {
			Debug.LogError ("Malformed server response \"" + readResult + "\"");
		} else {
			position_x_maf.add(parts[2]);
			position_y_maf.add(parts[0]);
			position_z_maf.add(parts[1]);

			rotation.w = parts[3];
			rotation.x = parts[4];
			rotation.y = parts[5];
			rotation.z = parts[6];
		}
	}
}
