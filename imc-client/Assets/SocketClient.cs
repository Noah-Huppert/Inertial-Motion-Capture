using UnityEngine;
using System;
using System.Text;
using System.Net.Sockets;

public delegate void ReadCallback(string result);

public class SocketClient {
	private TcpClient tcpClient;

	private string host;
	private int port;

	private const int READ_BUFFER_SIZE = 256;
	private byte[] readBuffer = new byte[READ_BUFFER_SIZE];

	private ReadCallback clientReadCallback;

	public SocketClient(string host, int port, ReadCallback clientReadCallback) {
		this.host = host;
		this.port = port;
		this.clientReadCallback = clientReadCallback;
	}

	public void connect() {
		if (tcpClient == null) {
			tcpClient = new TcpClient (host, port);
			Debug.Log ("Connected to socket " + host + ":" + port);
		}
	}

	public void disconnect() {
		if (tcpClient != null) {
			tcpClient.Close();
			Debug.Log ("Disconnected from socket " + host + ":" + port);
			tcpClient = null;
		}
	}

	public void write(string sendContent) {
		if (tcpClient != null && tcpClient.Connected) {
			int sendBufferSize = sendContent.Length;
			byte[] sendBuffer = new byte[sendBufferSize];
			sendBuffer = Encoding.UTF8.GetBytes(sendContent);

			tcpClient.GetStream().BeginWrite(sendBuffer, 0, sendBufferSize, new AsyncCallback(writeCallback), null);
		} else {
			Debug.Log("SocketClient must be connected");
		}
	}

	public void read() {
		if (tcpClient != null && tcpClient.Connected) {
			tcpClient.GetStream().BeginRead(readBuffer, 0, READ_BUFFER_SIZE, new AsyncCallback(readCallback), null);
		} else {
			Debug.Log("SocketClient must be connected");
		}
	}

	private void writeCallback(IAsyncResult asyncResult) {
		tcpClient.GetStream ().EndWrite (asyncResult);
	}

	private void readCallback(IAsyncResult asyncResult) {
		int bytesRead = tcpClient.GetStream ().EndRead (asyncResult);

		if (bytesRead < 1) {
			clientReadCallback("");
			return;
		}

		string readResult = Encoding.ASCII.GetString (readBuffer, 0, bytesRead - 2);

		clientReadCallback (readResult);
	}
}
