import io
import struct

class Stl:

    @staticmethod
    def read(filename):

        file = open(filename, "rb")

        file.seek(80)
        dreiecke = struct.unpack("I", file.read(4))[0]

        d = []
        for ii in range(dreiecke):
            file.seek(12, 1)
            dreieck = (struct.unpack("fff", file.read(12)),
                       struct.unpack("fff", file.read(12)),
                       struct.unpack("fff", file.read(12)))
            file.seek(2, 1)
            d.append(dreieck)

        return d
