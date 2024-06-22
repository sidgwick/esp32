import struct
import socket
import threading
import time

import cv2
import numpy as np


class ImageReceiverServer:
    def __init__(self, host, port):
        self.host = host
        self.port = port
        self.counter = 0

    def run(self):
        threads = []
        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
            s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
            s.bind((self.host, self.port))
            s.listen()

            print("Listening on {}:{}".format(self.host, self.port))

            while True:
                try:
                    conn, addr = s.accept()
                except KeyboardInterrupt:
                    print("Caught keyboard interrupt, exiting")
                    break

                print("Received from {}:".format(addr))

                t = ImageReceiver(conn)
                t.start()

                threads.append(t)

        for t in threads:
            t.join()


class ImageReceiver(threading.Thread):
    def __init__(self, conn: socket.socket):
        super().__init__()

        self.conn = conn
        self.counter = 0

    def run(self):
        try:
            while True:
                self._receiver_worker()
        except Exception as e:
            print("exception {}, close connection".format(e))
            self.conn.close()

    def _receiver_worker(self):
        conn = self.conn
        rcv = conn.recv(4)
        l = struct.unpack("!L", rcv)[0]

        print(f"try to read {l} from socket")
        image = b""
        while l > 0:
            tmp = conn.recv(1024 if l > 1024 else l)
            l -= len(tmp)
            image += tmp

        self.counter += 1
        img_name = "images/output_{}.png".format(self.counter)
        self.save_image(image, img_name)

        l = len(img_name)
        data = struct.pack("!L", l)
        conn.send(data)
        conn.send(img_name.encode())

    def save_image(self, image, img_name):
        print("[{}] image received".format(img_name))
        # show_image_gray(image, img_name)
        show_image_rgb565(image, img_name)
        # show_image_rgb888(image, img_name)


def rgb565_to_rgb888(rgb565):
    r = ((rgb565 >> 11) & 0x1F) * 8
    g = ((rgb565 >> 5) & 0x3F) * 4
    b = ((rgb565) & 0x1F) * 8
    return r, g, b


def show_image_gray(data, img_name):
    image = []

    for i in range(0, len(data)):
        pixel = data[i]
        image.append(pixel)

    image = np.array(image, dtype=np.uint8)
    image = image.reshape(240, 320, 1)
    cv2.imshow(img_name, image)
    cv2.waitKey(1)


def show_image_rgb888(data, img_name):
    image = []

    for i in range(0, len(data), 3):
        b = data[i : i + 3]
        r, g, b = b[0], b[1], b[2]
        image.extend([r, g, b])
        # image.append(r)

    image = np.array(image, dtype=np.uint8)
    image = image.reshape(240, 320, 3)
    image = cv2.cvtColor(image, cv2.COLOR_RGB2BGR)
    cv2.imshow(img_name, image)
    cv2.waitKey(1)


def show_image_rgb565(data, img_name):
    # raw = f"{img_name}.data"
    # with open(raw, "wb+") as fh:
    #     fh.write(data)

    image = []

    for i in range(0, len(data), 2):
        b = data[i : i + 2]
        rgb565 = struct.unpack("!H", b)[0]
        r, g, b = rgb565_to_rgb888(rgb565)
        image.extend([r, g, b])
        # image.append(r)

    image = np.array(image, dtype=np.uint8)
    image = image.reshape(240, 320, 3)
    image = cv2.cvtColor(image, cv2.COLOR_RGB2BGR)
    cv2.imwrite(img_name, image)


counter = 0
HOST, PORT = "0", 8458

# data = open(sys.argv[1], "rb").read()
# show_image(data)
# time.sleep(10)

ir = ImageReceiverServer(HOST, PORT)
ir.run()
