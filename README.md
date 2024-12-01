
# Plant Disease Detection - Backend

This folder contains the backend code for the **Plant Disease Detection** project. The backend is built using Flask and integrates a deep learning model for real-time plant disease detection.

---

## Setup Instructions

### 1. Download the Model
- Download the pre-trained model from the following link: [Google Drive - Model](https://drive.google.com/drive/folders/1JcDLEpwmKhi-_D4HEK705vJrivBHh1Fs?usp=sharing).

### 2. Create the Model Directory
- Create a directory named `model` in the `Backend` folder and place the downloaded model files into this directory:

   ```sh
   mkdir model
   ```

### 3. Install Dependencies
- Install the required Python packages listed in `requirements.txt`:

   ```sh
   pip install -r requirements.txt
   ```

### 4. Run the Flask Application
- Start the Flask application by running the following command in the terminal:

   ```sh
   python leafdisease.py
   ```

---

## Access the Video Feed
- Open your web browser and navigate to [http://localhost:5000/video_feed](http://localhost:5000/video_feed) to view the real-time video feed with plant disease detection.

---

## ESP32 Integration

Outside the `Backend` folder, there is an `ESP32` folder used for programming the ESP32 microcontroller and connecting it to **Blynk**.

### Setup Instructions for ESP32

1. **Create `secrets.h`**
   - Inside the `src` folder of the `ESP32` project, create a file named `secrets.h` with the following content:

   ```cpp
   #ifndef SECRETS_H
   #define SECRETS_H

   #define WIFI_SSID "YourWiFiSSID"
   #define WIFI_PASSWORD "YourWiFiPassword"
   #define BLYNK_TEMPLATE_ID "YourBlynkTemplateID"
   #define BLYNK_TEMPLATE_NAME "YourBlynkTemplateName"
   #define BLYNK_AUTH_TOKEN "YourBlynkAuthToken"
   #define SERVER_ADDRESS "YourServerAddress"
   #define SERVER_PORT YourServerPort
   #define APPSCRIPTS_URL "https://script.google.com/macros/s/YOUR_APPSCRIPTS_WEBAPP_URL/exec"
   #define THINGESP_USERNAME "YourThingESPUsername"
   #define TWILILO_PROJECTNAME "YourTwilioProjectName"
   #define TWILILO_DEVICENAME "YourTwilioDeviceName"

   #endif // SECRETS_H
   ```

2. **Install Necessary Libraries**
   - Use **PlatformIO** to manage and upload the code. Ensure all required libraries are installed in the `platformio.ini` file.

3. **Upload the Code**
   - Connect your ESP32 to the computer and upload the code using PlatformIO.

---

## Model and Dataset

### Model: MobileNet-SSD
- The project utilizes the **MobileNet-SSD** model
- Repository: [MobileNet-SSD](https://github.com/chuanqi305/MobileNet-SSD).

### Dataset: PlantVillage
- The training dataset used for plant disease detection is the **PlantVillage Dataset**
- Repository: [PlantVillage Dataset](https://github.com/spMohanty/PlantVillage-Dataset).

---

## File Structure

- **`leafdisease.py`**: Main application file containing the Flask server and functions for detecting diseases in plants.
- **`model/`**: Directory containing the pre-trained models.
- **`requirements.txt`**: File listing the dependencies required for the project.
- **`ESP32/`**: Folder for ESP32 firmware with the `src` directory to include the `secrets.h` file.

---

## Dependencies

This project relies on the following Python libraries:
- **OpenCV**: For video capture and processing.
- **NumPy**: For numerical computations.
- **Keras**: For deep learning model integration.
- **TensorFlow**: Backend for the Keras model.
- **Flask**: For building the server application.
- **Flask-CORS**: To enable Cross-Origin Resource Sharing.

---

## Notes

- Ensure your camera is connected and accessible by OpenCV before starting the application.
- You can adjust the confidence threshold and other parameters in the `leafdisease.py` file to improve detection performance.
- For ESP32, ensure your WiFi credentials and Blynk details are correctly configured in the `secrets.h` file.
