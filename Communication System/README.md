# Communication System Design using Sound

> This project is a (mini-) communication system with sends and recieves bit-strings between host and client using sound.

You may want to check out the [report.pdf](report.pdf) file for more details. It was made as a part of the CS252 (Computer Networks Lab) course requirement at IIT Bombay.

## Getting Started

Follow the instructions below to get our project running on your local machine.

1. Clone the repository and run the [main.py](src/main.py) file interatively in python3
2. Now, start the `client()` and `host()` functions on respective devices to start communication.
3. `host()` function takes up a `list` of messages (of type `str`) as arguments and sends them to the client.

### Prerequisites

PyAudio - Install PyAudio from PyPI using pip3 command.

```
pip3 install pyaudio
```

_(You may need to install any additional dependencies for pyaudio)_

## Built With

* Python3 - The complete project is coded in python3
* [PyAudio](https://pypi.python.org/pypi/PyAudio) - The python package to stream audio input/output.

## Authors

* **Vamsi Krishna Reddy Satti** - *Initial work* - [vamsi3](https://github.com/vamsi3)
* Vighnesh Reddy Konda - [scopegeneral](https://github.com/scopegeneral)
* Lakshmi Narayana Chappidi
* Yaswanth Kumar Orru

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details

## Acknowledgements

* Thanks to anyone who's code was used.