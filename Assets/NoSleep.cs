using UnityEngine;
using System.Collections;

public class NoSleep : MonoBehaviour {

	// This class keeps your phone alive so it can continuously send data
	void Start () {
		Screen.sleepTimeout = SleepTimeout.NeverSleep;
	}
}
