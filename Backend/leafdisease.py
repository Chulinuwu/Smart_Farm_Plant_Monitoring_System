# leaf_disease_detection.py
import cv2
import numpy as np
from keras.models import load_model
import tensorflow as tf
from flask import Flask, Response
import time
from flask_cors import CORS

# Load the pre-trained regression model
model = load_model('model/plant_disease_regression_model.h5')

# Load the MobileNet SSD model
net = cv2.dnn.readNetFromCaffe('model/deploy.prototxt', 'model/mobilenet_iter_73000.caffemodel')

app = Flask(__name__)
CORS(app)

def detect_people(frame):
    blob = cv2.dnn.blobFromImage(frame, 0.007843, (300, 300), 127.5)
    net.setInput(blob)
    detections = net.forward()

    people_boxes = []
    for i in range(detections.shape[2]):
        confidence = detections[0, 0, i, 2]
        if confidence > 0.2:
            idx = int(detections[0, 0, i, 1])
            if idx == 15:
                box = detections[0, 0, i, 3:7] * np.array([frame.shape[1], frame.shape[0], frame.shape[1], frame.shape[0]])
                (startX, startY, endX, endY) = box.astype("int")
                people_boxes.append((startX, startY, endX, endY))
    return people_boxes

def predict_disease(frame):
    image_resized = cv2.resize(frame, (256, 256))
    image_array = np.array(image_resized)
    image_array = np.expand_dims(image_array, axis=0)

    predictions = model.predict(image_array)
    probability = predictions[0][0]

    return probability

def generate_frames():
    cap = cv2.VideoCapture(0)
    if not cap.isOpened():
        print("Error: Could not open camera.")
        return

    while True:
        ret, frame = cap.read()
        if not ret:
            print("Error: Could not read frame.")
            break

        people_boxes = detect_people(frame)
        if not people_boxes:
            probability = predict_disease(frame)
            healthy_prob = 1 - probability
            diseased_prob = probability

            if diseased_prob > 0:
                label = f'Diseased Probability: {diseased_prob:.2f}'
                color = (0, 0, 255)
            else:
                label = f'Healthy Probability: {healthy_prob:.2f}'
                color = (0, 255, 0)

            cv2.rectangle(frame, (0, 0), (frame.shape[1], frame.shape[0]), color, 2)
            cv2.putText(frame, label, (10, 30), cv2.FONT_HERSHEY_SIMPLEX, 0.6, color, 2)

        ret, buffer = cv2.imencode('.jpg', frame)
        frame = buffer.tobytes()
        yield (b'--frame\r\n'
               b'Content-Type: image/jpeg\r\n\r\n' + frame + b'\r\n')

@app.route('/video_feed')
def video_feed():
    return Response(generate_frames(), mimetype='multipart/x-mixed-replace; boundary=frame')

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000)