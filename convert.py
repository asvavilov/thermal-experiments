# конвертация для чтения и печати с sd-карты

from PIL import Image

img = Image.open("input.jpg").convert("L")
# Приводим к ширине печати принтера: 384 точки (48 мм)
img = img.resize((384, max(1, round(img.height * 384 / img.width))))
# Инвертируем: темные → 255 (печатать), светлые → 0 (не печатать)
img = img.point(lambda x: 255 if x < 128 else 0, "1")

width, height = img.size
raw = img.tobytes()

with open("image.raw", "wb") as f:
    f.write((width).to_bytes(2, byteorder='little'))
    f.write((height).to_bytes(2, byteorder='little'))
    f.write(raw)

print(f"image.raw: {width}x{height}")
