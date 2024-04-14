import cv2
import numpy as np
import tflite_runtime.interpreter as tflite
from picamera2.picamera2 import Picamera2
import time
from data import person_from_keypoints_with_scores
from utils import keep_aspect_ratio_resizer
import websocket

try:
    ws = websocket.create_connection("ws://192.168.1.100:81")
except Exception as e:
    print("Fail to connect to the WebSocket:", e)
    ws = None

picam2 = Picamera2()
config = picam2.create_preview_configuration()
picam2.configure(config)
picam2.start()

movenet_interpreter = tflite.Interpreter(model_path="movenet_thunder.tflite")
movenet_interpreter.allocate_tensors()

pose_classifier_interpreter = tflite.Interpreter(model_path="pose_classifier.tflite")
pose_classifier_interpreter.allocate_tensors()

person_score_threshold = 0.6
status_start_time = None
current_status = None
status_duration_threshold = 5

preview_message_sent = False
analysis_started = False
start_time = time.time()

try:
    while True:
        current_time = time.time()
        frame = picam2.capture_array()
        preprocessed_image = keep_aspect_ratio_resizer(frame, target_size=256)
        if preprocessed_image.dtype != np.uint8:
            preprocessed_image = preprocessed_image.astype(np.uint8)
        cv2.imshow('Preprocessed Image', preprocessed_image)

        if current_time - start_time < 10:
            if not preview_message_sent and ws:
                ws.send("start preview")
                preview_message_sent = True
        else:
            if not analysis_started and ws:
                ws.send("start analysis")
                analysis_started = True
            
            input_image = np.expand_dims(preprocessed_image, axis=0)
            movenet_interpreter.set_tensor(movenet_interpreter.get_input_details()[0]['index'], input_image)
            movenet_interpreter.invoke()
            keypoints_with_scores = movenet_interpreter.get_tensor(movenet_interpreter.get_output_details()[0]['index'])[0]

            person_score = np.max(keypoints_with_scores[:, 2])
            if person_score >= person_score_threshold:
                prepared_landmarks = keypoints_with_scores.flatten().astype('float32')
                pose_classifier_interpreter.set_tensor(pose_classifier_interpreter.get_input_details()[0]['index'], [prepared_landmarks])
                pose_classifier_interpreter.invoke()
                classification_results = pose_classifier_interpreter.get_tensor(pose_classifier_interpreter.get_output_details()[0]['index'])[0]

                new_status = "good posture" if classification_results[1] > classification_results[0] else "bad posture"
                if new_status != current_status:
                    current_status = new_status
                    status_start_time = time.time()
                elif time.time() - status_start_time > status_duration_threshold:
                    if ws:
                        ws.send(current_status)
                    status_start_time = time.time()
            else:
                if current_status != "no human detected":
                    current_status = "no human detected"
                    status_start_time = time.time()
                elif time.time() - status_start_time > status_duration_threshold:
                    if ws:
                        ws.send("no human detected")
                    status_start_time = time.time()

        if cv2.waitKey(1) & 0xFF == ord('q'):
            break
finally:
    picam2.stop()
    cv2.destroyAllWindows()
    if ws:
        ws.close()
