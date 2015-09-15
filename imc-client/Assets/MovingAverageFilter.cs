using UnityEngine;
using System.Collections.Generic;

public class MovingAverageFilter {
	private int size;
	private List<float> list = new List<float>();

	public MovingAverageFilter(int size) {
		this.size = size;
	}

	public void add(float data) {
		list.Insert (0, data);

		while (list.Count > size) {
			list.RemoveAt(list.Count - 1);
		}
	}

	public float average() {
		float total = 0;

		for (int i = 0; i < list.Count; i++) {
			total += list [i];
		}

		return total / list.Count;
	}
}
