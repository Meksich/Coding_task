
import serial
import time
from PySide6 import QtCore, QtWidgets
from PySide6.QtGui import QFont


class Widget(QtWidgets.QWidget):
    def __init__(self):
        self.led_blinking = True
        self.is_base_frequency = True
        super().__init__()
        self.ser = serial.Serial('COM3', 115200, timeout=1)
        time.sleep(2)
        self.button_on = QtWidgets.QPushButton("Turn on LED!")
        self.button_off = QtWidgets.QPushButton("Turn off LED!")
        self.button_blink = QtWidgets.QPushButton("Start blinking LED!")
        self.button_change_frequency = QtWidgets.QPushButton("Change frequency to 10Hz!")
        self.buttons = [self.button_change_frequency, self.button_off, self.button_on, self.button_blink]
        self.text = QtWidgets.QLabel("Welcome to simple UART app",
                                     alignment=QtCore.Qt.AlignHCenter)
        self.layout = QtWidgets.QVBoxLayout(self)
        self.ui_setup()

    @QtCore.Slot()
    def led_on(self):
        self.led_blinking = False
        self.button_change_frequency.setEnabled(self.led_blinking)
        self.ser.write(b'o')   # send a byte

    @QtCore.Slot()
    def led_off(self):
        self.led_blinking = False
        self.button_change_frequency.setEnabled(self.led_blinking)
        self.ser.write(b'f')

    @QtCore.Slot()
    def led_blink(self):
        self.led_blinking = True
        self.button_change_frequency.setEnabled(self.led_blinking)
        self.ser.write(b'b')

    @QtCore.Slot()
    def led_change_frequency(self):
        if self.is_base_frequency:
            self.button_change_frequency.setText("Change frequency to 0.5Hz!")
        else:
            self.button_change_frequency.setText("Change frequency to 10Hz!")
        self.is_base_frequency = not self.is_base_frequency
        self.ser.write(b'c')

    def ui_setup(self):
        self.button_on.setGeometry(400,400,300,260)
        self.layout.addWidget(self.text)
        self.layout.addWidget(self.button_on)
        self.layout.addWidget(self.button_off)
        self.layout.addWidget(self.button_blink)
        self.layout.addWidget(self.button_change_frequency)
        self.button_on.clicked.connect(self.led_on)
        self.button_off.clicked.connect(self.led_off)
        self.button_blink.clicked.connect(self.led_blink)
        self.button_change_frequency.clicked.connect(self.led_change_frequency)
