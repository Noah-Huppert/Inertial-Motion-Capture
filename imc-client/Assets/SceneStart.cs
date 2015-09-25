using UnityEngine;
using System.Collections;

public class SceneStart : MonoBehaviour {
    public GameObject camera;
    public GameObject title_group;
    public GameObject catapult;
    public GameObject ball;

    private bool started = false;
    private bool stage_title_position = false;
    private bool stage_catapult_scale = false;
    private bool stage_catapult_rotation = false;
    private bool stage_camera_position = false;
    private bool stage_camera_rotation = false;

    private float title_group_y_target = 85;
    private float catapult_rotation_y_target = 0;
    private Vector3 catapult_scale_target = new Vector3(2, 2, 2);
    private Vector3 camera_position_target = new Vector3(0, 29.8F, -24.3F);
    private Vector3 camera_rotation_target = new Vector3(23.19F, 0, 0);

	void Start () {
	
	}
	
	void Update () {
	    if(Input.GetKeyUp(KeyCode.Space)) {
            started = true;
        }

        // Check
        if(started && !stage_title_position && title_group.transform.position.y >= 83) {
            stage_title_position = true;
        }

        if(started && !stage_catapult_scale && catapult.transform.localScale.x >= 1.9 &&
                           catapult.transform.localScale.y >= 1.9 &&
                           catapult.transform.localScale.z >= 1.9) {
            stage_catapult_scale = true;
        }

        if(started && !stage_catapult_rotation && catapult.transform.rotation.eulerAngles.y >= 359) {
            stage_catapult_rotation = true;
        }

        if(started && !stage_camera_position && camera.transform.position.y >= 29F &&
                                     camera.transform.position.z <= -24F) {
            stage_camera_position = true;
        }

        if(started && !stage_camera_rotation && camera.transform.rotation.eulerAngles.x >= 23F) {
            stage_camera_rotation = true;
        }

        if(stage_title_position && stage_catapult_scale && stage_catapult_rotation && stage_camera_position && stage_camera_rotation) {
            Application.LoadLevel("Demo-Game");
        }

        // Slerp
        if(started && !stage_title_position) {
            Vector3 title_group_pos = title_group.transform.position;
            title_group_pos.y = title_group_y_target;
            slerp_position(title_group, title_group_pos);
        }

        if(started && !stage_catapult_scale) {
            catapult.transform.localScale = Vector3.Slerp(
                catapult.transform.localScale,
                catapult_scale_target,
                Time.deltaTime * 2F
            );
        }

        if(started && !stage_catapult_rotation) {
            slerp_rotation(catapult, new Vector3(0, catapult_rotation_y_target, 0));
        }

        if(started && !stage_camera_position) {
            slerp_position(camera, camera_position_target);
        }

        if(started && !stage_camera_rotation) {
            slerp_rotation(camera, camera_rotation_target);
        }
	}

    private void slerp_position(GameObject obj, Vector3 target) {
        obj.transform.position = Vector3.Slerp(
            obj.transform.position,
            target,
            Time.deltaTime * 2F
        );
    }

    private void slerp_rotation(GameObject obj, Vector3 target) {
        obj.transform.rotation = Quaternion.Slerp(
            obj.transform.rotation,
            Quaternion.Euler(target),
            Time.deltaTime * 2F
        );
    }
}
