import io

class Header:

    def export(template_file, target_file, matrix):

        res_t = len(matrix)
        res_y = len(matrix[0])

        imgdata = "{\n"
        for t in range(res_t):

            imgdata += "{0b"

            for y in range(8):
                imgdata += str(matrix[t][y])

            for ii in range(1, 4):
                imgdata += ", 0b"

                for x in range(8):
                    imgdata += str(matrix[t][ii * 8 + x])

            imgdata += "},\n"

        imgdata = imgdata[:-2] + "}"

        print imgdata

        f = open(template_file, "r")
        content = f.read()
        f.close()

        content = content.replace("$IMGDATA", imgdata)
        content = content.replace("$RES_Y", str(res_y))
        content = content.replace("$RES_T", str(res_t))

        f = open(target_file, "w")
        f.write(content)
        f.close()
