using UnityEngine;
using System;
using System.Collections;
using System.IO;
using System.Net;
using System.Net.Sockets;
using System.Text;

public class SocketEchoController : MonoBehaviour {
	private GameObject socketCube;
	private SocketClient socketClient;

	private float lastUpdateTime = 0;
	private float updateTimeInterval = 0.001F;

	private Vector3 socketCubePosition;

	void Start() {
		socketCube = GameObject.Find ("SocketCube");

		socketClient = new SocketClient("192.168.1.9", 1234, socketReadCallback);

		socketClient.connect ();
	}

	void Update() {
		if (Math.Abs(Time.time - lastUpdateTime) >= updateTimeInterval) {
			lastUpdateTime = Time.time;

			Vector3 position = transform.position;
			string positionString = position.x + " " + position.y + " " + position.z;
			socketClient.write (positionString);
			socketClient.read ();

			socketCube.transform.position = socketCubePosition;
		}
	}

	void onDestroy() {
		socketClient.write ("EXIT");
	}

	void socketReadCallback(string readResult) {
		if (readResult == "EXIT") {
			socketClient.disconnect ();
			return;
		}

		string[] posParts = readResult.Split (" "[0]);

		socketCubePosition.x = float.Parse (posParts [0]);
		socketCubePosition.y = float.Parse (posParts [1]) + 1.5F;
		socketCubePosition.z = float.Parse (posParts [2]);
	}
}
