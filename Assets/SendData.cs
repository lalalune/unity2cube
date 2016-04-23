using UnityEngine;
using System.Collections;

using System;
using System.Text;
using System.Net;
using System.Net.Sockets;
using System.Threading;
using SimpleJSON;

public class SendData : MonoBehaviour
{

	// ATTATCH ONE OF THESE SCRIPTS FOR EACH CUBE YOU WANT TO SEND TO
	// Then adjust the X, Y and Z offsets accordingly. So if you have two cubes next to each other
	// One will have an X offset of 16 (or 8 for a small cube)
	// If you had 8 cubed up, you would give them X, Y and Z offsets, respectively
	// The debug.DrawLine will so you where your cubes are hitting, so you can grid it out.

	private int localPort;

	// prefs

	public string token;
	public string deviceID;

	string http = "https://api.particle.io/v1/devices/";
	string http2 = "/IPAddress?access_token=";

	public string IP;  // define in init
	public int port;  // define in init

	// These colors are 8 bit values (2-bit RGB color). Bits 1 and 2 are red, bits 3 and 4 are green, 5 and 6 are blue, 7 and 8 are nothing but could be used for brightness
	// An int of value 64 (1 0 0 0 0 0 0 0) would be red. 68 (1 0 0 0 0 0 1 0 0) would be red with some blue. Etc etc.
	// We're working on a better way to encode this

	public byte color1;
	public byte color2;
	public byte color3;
	public byte color4;


	public byte empty = 0;

	// Offset the starting point of your raycasts, so you can have multiple cubes

	public int xOffset = 0;
	public int yOffset = 0;
	public int zOffset = 0;


	// Wait to start sending data
	bool readyToSend = false;



	// Timer to check the Spark to see if it's IP address settings have changed. If the Particle crashes or restarts, this *should* allow Unity to find it again
	float aliveTimer = 0;
	bool started = false;
	int counter;

	// How many times per second you want to send data.
	public float FPS = 10;
	float timer = 0;

	// If you want to use a smaller cube, you can set these values to 8 and 512 respectively. The big cube has 16 per side for 4096 total LEDs, hence the 4096.

	public int numPerSide = 16;
	public int values = 4096;

	// create an output array
	byte[] output = new byte[4096];

	RaycastHit hit;


	// "connection" things
	IPEndPoint remoteEndPoint;
	UdpClient client;


	void Update(){

		if(readyToSend == true){

		timer += Time.deltaTime;

			if (timer >= 1f / FPS) {

				timer -= timer;

				getPixels ();

			}

			aliveTimer += Time.deltaTime;

			if (timer >= 30f) {
				timer -= timer;
			}

		}




	}


	public void Start()
	{
		Debug.Log ("Did you put in your Device ID and Token? Remove this line when you do :)");
		http = http + deviceID + http2 + token;
		StartCoroutine(findCubes ());
		// This will draw your field of capture
		for (int x = 0; x < numPerSide; x++){
			for (int y = 0; y < numPerSide; y++) {
				Debug.DrawRay (new Vector3 (x + xOffset, y + yOffset, 0f + zOffset), transform.forward, Color.red, 32000, true);

			}
		}

		}


	// This checks for the variables on Spark. MAKE SURE you enter your token and device ID!

	IEnumerator findCubes()
	{
		WWW www = new WWW(http);
		yield return www;
		if (www.error == null)
		{
			var parsedJSON = JSON.Parse(www.data);
			yield return null;
			IP = parsedJSON ["result"].Value;
			if (started == false) {
				started = true;
				yield return new WaitForSeconds (1);
				init ();
				readyToSend = true;
			}
			yield return null;
		}
		else
		{
			Debug.Log("ERROR: " + www.error);
		} 

	}    



	// init
	public void init()
	{
		// ----------------------------
		// Send  the data
		// ----------------------------
		remoteEndPoint = new IPEndPoint(IPAddress.Parse(IP), port);
		client = new UdpClient();
		// status
		print("Sending to "+IP+" : "+port);
		print("Testing: nc -lu "+IP+" : "+port);



	}


	void getPixels(){

		// This is where we do the magic of getting our grid. We raycast from all directions (except for bottom, go ahead and do that if you want)
		// Yes, this could/should all be wrapped into a function and simplified heavily
		// There is a bug where sometimes the pixels will pop up on the opposite side. If you fix this, please e-mail the fix to me! shaw@bravomedia.com


		// Clear al the data
		for (int i = 0; i < 4096; i++) {
			output [i] = 0; }

//		// back
		for (int x = 0; x < numPerSide; x++){
			for (int y = 0; y < numPerSide; y++) {
				Ray ray = new Ray(new Vector3(x + xOffset, y + yOffset, numPerSide + zOffset), transform.forward);
				if (Physics.Raycast (ray, out hit, numPerSide)) {
					int z = (int)Mathf.Clamp (Mathf.Round (hit.distance), 0, numPerSide);

					byte outColor = 0;
					if (hit.collider.tag == "color1"){
						outColor = color1;
					}
					else if (hit.collider.tag == "color2"){
						outColor = color2;
					}
					else if (hit.collider.tag == "color3"){
						outColor = color3;
					}

					else if (hit.collider.tag == "color4"){
						outColor = color4;
					}

					output [Mathf.Clamp(x + (y * numPerSide) + (z * numPerSide * numPerSide), 0, values - 1)] = outColor;

				}
			}
		}
		// left side
		for (int y = 0; y < numPerSide; y++){
			for (int z = 0; z < numPerSide; z++) {
				Ray ray = new Ray(new Vector3(0 + xOffset, y + yOffset, numPerSide + zOffset), new Vector3(1,0,0));
				if (Physics.Raycast (ray, out hit, numPerSide)) {
					int x = (int)Mathf.Clamp (Mathf.Round (hit.distance), 0, numPerSide);

					byte outColor = 0;
					if (hit.collider.tag == "color1"){
						outColor = color1;
					}
					else if (hit.collider.tag == "color2"){
						outColor = color2;
					}
					else if (hit.collider.tag == "color3"){
						outColor = color3;
					}

					else if (hit.collider.tag == "color4"){
						outColor = color4;
					}

					output [Mathf.Clamp(x + (y * numPerSide) + (values - z * numPerSide * numPerSide), 0, values - 1)] = outColor;

				}
			}
		}

		//right

		for (int y = 0; y < numPerSide; y++){
			for (int z = 0; z < numPerSide; z++) {
				Ray ray = new Ray(new Vector3(numPerSide + xOffset, y + yOffset, z + zOffset), new Vector3(-1,0,0));
				if (Physics.Raycast (ray, out hit, numPerSide)) {
					int x = (int)Mathf.Clamp (Mathf.Round (hit.distance), 0, numPerSide);

					byte outColor = 0;
					if (hit.collider.tag == "color1"){
						outColor = color1;
					}
					else if (hit.collider.tag == "color2"){
						outColor = color2;
					}
					else if (hit.collider.tag == "color3"){
						outColor = color3;
					}

					else if (hit.collider.tag == "color4"){
						outColor = color4;
					}

					output [Mathf.Clamp(numPerSide - x + (y * numPerSide) + (values - z * numPerSide * numPerSide), 0, values - 1)] = outColor;

				}
			}
		}

		// top down

		for (int x = 0; x < numPerSide; x++){
			for (int z = 0; z < numPerSide; z++) {
				Ray ray = new Ray(new Vector3(x + xOffset, numPerSide + yOffset, z + zOffset), new Vector3(0,-1,0));
				if (Physics.Raycast (ray, out hit, numPerSide)) {
					int y = (int)Mathf.Clamp (Mathf.Round (hit.distance), 0, numPerSide);

					byte outColor = 0;
					if (hit.collider.tag == "color1"){
						outColor = color1;
					}
					else if (hit.collider.tag == "color2"){
						outColor = color2;
					}
					else if (hit.collider.tag == "color3"){
						outColor = color3;
					}

					else if (hit.collider.tag == "color4"){
						outColor = color4;
					}

					output [Mathf.Clamp(x + (numPerSide * numPerSide - y * numPerSide) + (values - z * numPerSide * numPerSide), 0, values - 1)] = outColor;

				}
			}
		}


		// front
		for (int x = 0; x < numPerSide; x++){
			for (int y = 0; y < numPerSide; y++) {
				Ray ray = new Ray(new Vector3(x + xOffset, y + yOffset, 0 + zOffset), transform.forward);
				if (Physics.Raycast (ray, out hit, numPerSide)) {
					int z = (int)Mathf.Clamp (Mathf.Round (hit.distance), 0, numPerSide);

					byte outColor = 0;
					if (hit.collider.tag == "color1"){
						outColor = color1;
					}
					else if (hit.collider.tag == "color2"){
						outColor = color2;
					}
					else if (hit.collider.tag == "color3"){
						outColor = color3;
					}

					else if (hit.collider.tag == "color4"){
						outColor = color4;
					}

					output [Mathf.Clamp(x + y * numPerSide + (values - z * numPerSide * numPerSide), 0, values - 1)] = outColor;

						}
			}
		}

		
		// Now we've got our data, let's send it!
		client.Send(output, 4096, remoteEndPoint);


	}



}