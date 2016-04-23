using UnityEngine;
using System.Collections;

public class Move : MonoBehaviour {

	public bool moveUp = false;
	public bool moveForward = false;
	public float distance;
	public float offset;
	public float speed;

	// This is just a fun script to make our object move back and forth

	void Start () {
	
	}
	
	void Update () {
		if (moveUp == false) {
			transform.position = new Vector3 (Mathf.PingPong (Time.time * speed, distance) - offset, transform.position.y, transform.position.z);
		} else {
			if (moveForward == false) {

				transform.position = new Vector3 (transform.position.x, Mathf.PingPong (Time.time * speed, distance) - offset, transform.position.z);
			} else {
				transform.position = new Vector3 (transform.position.x, transform.position.y, Mathf.PingPong (Time.time * speed, distance) - offset);

			}

		}
		
		}
}
