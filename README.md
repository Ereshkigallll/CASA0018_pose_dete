# CASA0018 Posture Guard

## 1. Introduction

Do you often feel muscle soreness around your neck and shoulders or discomfort in your lower back after long periods of work? Try my creation, the Posture Guard! This is a fully offline AI posture recognition system deployed on a Raspberry Pi. All you need to do is place the Raspberry Pi and its camera beside your seat and set up the ESP8266 along with the LED matrix screen wherever you like, as long as it's visible to you. It’s ready to start working! I hope it helps you sit healthier!

![pic1](https://github.com/Ereshkigallll/CASA0018_pose_dete/blob/main/pic/478f5e3ec835774dc9ee56108bd8fa0.jpg)

## 2. Requirement
- Raspberry Pi
- ESP8266
- CSI camera or USB camera
- LED matrix screen

## 3. How to Use

### 3.1 ESP8266
Just upload the file in the `pose_esp` folder to your ESP8266, and record the printed WebSocket address.

### 3.1 Create Virtual Environment
First, use the following command to clone the repository:

```
git clone https://github.com/Ereshkigallll/CASA0018_pose_dete.git
cd CASA0018_pose_dete
```

Since installing certain Python libraries directly on the Raspberry Pi can be problematic, it is recommended to circumvent this by using a virtual environment:

Let's create a virtual environment first!

```
python3 -m venv pose_dete
```

Then we need to activate it!

```
source pose_dete/bin/activate
```

If you want to quit this environment, use the command below:

```
deactivate
```

Once that's done, we can start installing the python libraries we need for this project!

### 3.2 Installing Useful Libraries and Download Some Useful File

Use following command to install some useful python libraries for this project:

```
sudo apt-get install python3-opencv
pip3 install numpy
pip3 install --extra-index-url https://google-coral.github.io/py-repo/ tflite_runtime
pip3 install picamera2
pip3 install websocket-client
```

 
In addition to this, you will need to download the following two files in this [Tensorflow sample project](https://github.com/tensorflow/examples/tree/master/lite/examples/pose_estimation/raspberry_pi): `data.py` and `utils.py`.

Finally make sure that the two model files and the two files you just downloaded are in the RPi folder.

After confirming that you have the same WebSocket address for the ESP8266 and the file `pi.py` on the Raspberry Pi, you can use the following commands to start running:

```
python pi.py
```

## 4. If You Want to Train Your Own Model...

I have provided you with very detailed instructions in `posture.ipynb`, so you can get your own sitting posture classification model if you follow along.

During my data collection process, I collected a total of 1000 images, 500 for good sitting posture as well as 500 for bad sitting posture. The ratio of training set, validation set and test set is 8:1:1.

All photos are converted to 256*256 resolution by scaling and padding operations to meet MoveNet's data input requirements.

After all the images are fed into MoveNet, according to the code MoveNet will output a CSV file containing the horizontal and vertical coordinates of the 17 key points of the human body in the image as well as the confidence scores, and my sitting posture classification model is trained on this `CSV` file.

This classification model training has low hardware requirements. Since the low batch size during training ensures that the model has a high accuracy, it has low running memory as well as graphics requirements (if you are using a GPU for training).

Instead, it takes relatively longer when converting images to coordinate point files via MoveNet, and a Nvidia GPU really helps a lot!