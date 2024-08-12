/* eslint-disable */
import {
  DefaultLoadingManager, Texture, FileLoader,
} from 'three'

const TGALoader = function (manager) {
  this.manager = (manager !== undefined) ? manager : DefaultLoadingManager
}

TGALoader.prototype = {

  constructor: TGALoader,

  load (url, onLoad, onProgress, onError) {
    var scope = this

    var texture = new Texture()

    var loader = new FileLoader(this.manager)
    loader.setResponseType('arraybuffer')

    loader.load(url, function (buffer) {
      texture.image = scope.parse(buffer)
      texture.needsUpdate = true
      if (onLoad !== undefined) {
        onLoad(texture)
      }
    }, onProgress, onError)
    return texture
  },

  parse: function (buffer) {
    // reference from vthibault, https://github.com/vthibault/roBrowser/blob/master/src/Loaders/Targa.js

    function tgaCheckHeader (header) {
      // check indexed type
      switch (header.image_type) {
      case TGA_TYPE_INDEXED:
      case TGA_TYPE_RLE_INDEXED:
        if (header.colormap_length > 256 || header.colormap_size !== 24 || header.colormap_type !== 1) {
          console.error('TGALoader: Invalid type colormap data for indexed type.')
        }
        break

        // check colormap type

      case TGA_TYPE_RGB:
      case TGA_TYPE_GREY:
      case TGA_TYPE_RLE_RGB:
      case TGA_TYPE_RLE_GREY:
        if (header.colormap_type) {
          console.error('TGALoader: Invalid type colormap data for colormap type.')
        }
        break

        // What the need of a file without data ?

      case TGA_TYPE_NO_DATA:
        console.error('TGALoader: No data.')
        // Invalid type ?
        break
      default:
        console.error('TGALoader: Invalid type "%s".', header.image_type)
      }

      // check image width and height

      if (header.width <= 0 || header.height <= 0) {
        console.error('TGALoader: Invalid image size.')
      }

      // check image pixel size

      if (header.pixel_size !== 8 && header.pixel_size !== 16 &&
        header.pixel_size !== 24 && header.pixel_size !== 32) {
        console.error('TGALoader: Invalid pixel size "%s".', header.pixel_size)
      }
    }

    // parse tga image buffer

    function tgaParse (useRle, usePal, header, offset, data) {
      var pixelData,
        pixelSize,
        pixelTotal,
        palettes
      pixelSize = header.pixel_size >> 3
      pixelTotal = header.width * header.height * pixelSize
      // read palettes
      if (usePal) {
        palettes = data.subarray(offset, offset += header.colormap_length * (header.colormap_size >> 3))
      }

      // read RLE

      if (useRle) {
        pixelData = new Uint8Array(pixelTotal)
        var c, count, i
        var shift = 0
        var pixels = new Uint8Array(pixelSize)

        while (shift < pixelTotal) {
          c = data[offset++]
          count = (c & 0x7f) + 1

          // RLE pixels

          if (c & 0x80) {
            // bind pixel tmp array

            for (i = 0; i < pixelSize; ++i) {
              pixels[i] = data[offset++]
            }

            // copy pixel array

            for (i = 0; i < count; ++i) {
              pixelData.set(pixels, shift + i * pixelSize)
            }
            shift += pixelSize * count
          } else {
            // raw pixels
            count *= pixelSize
            for (i = 0; i < count; ++i) {
              pixelData[shift + i] = data[offset++]
            }
            shift += count
          }
        }
      } else {
        // raw pixels
        pixelData = data.subarray(
          offset, offset += (usePal ? header.width * header.height : pixelTotal),
        )
      }

      return {
        pixel_data: pixelData,
        palettes: palettes,
      }
    }

    function tgaGetImageData8bits (imageData, yStart, yStep, yEnd, xStart, xStep, xEnd, image, palettes) {
      var colormap = palettes
      var color
      var i = 0
      var x
      var y
      var width = header.width

      for (y = yStart; y !== yEnd; y += yStep) {
        for (x = xStart; x !== xEnd; x += xStep, i++) {
          color = image[i]
          imageData[(x + width * y) * 4 + 3] = 255
          imageData[(x + width * y) * 4 + 2] = colormap[color * 3]
          imageData[(x + width * y) * 4 + 1] = colormap[(color * 3) + 1]
          imageData[(x + width * y) * 4] = colormap[(color * 3) + 2]
        }
      }

      return imageData
    }

    function tgaGetImageData16bits (imageData, yStart, yStep, yEnd, xStart, xStep, xEnd, image) {
      var color
      var i = 0
      var x
      var y
      var width = header.width

      for (y = yStart; y !== yEnd; y += yStep) {
        for (x = xStart; x !== xEnd; x += xStep, i += 2) {
          color = image[i] + (image[i + 1] << 8) // Inversed ?
          imageData[(x + width * y) * 4] = (color & 0x7C00) >> 7
          imageData[(x + width * y) * 4 + 1] = (color & 0x03E0) >> 2
          imageData[(x + width * y) * 4 + 2] = (color & 0x001F) >> 3
          imageData[(x + width * y) * 4 + 3] = (color & 0x8000) ? 0 : 255
        }
      }

      return imageData
    }

    function tgaGetImageData24bits (imageData, yStart, yStep, yEnd, xStart, xStep, xEnd, image) {
      var i = 0
      var x
      var y
      var width = header.width

      for (y = yStart; y !== yEnd; y += yStep) {
        for (x = xStart; x !== xEnd; x += xStep, i += 3) {
          imageData[(x + width * y) * 4 + 3] = 255
          imageData[(x + width * y) * 4 + 2] = image[i]
          imageData[(x + width * y) * 4 + 1] = image[i + 1]
          imageData[(x + width * y) * 4] = image[i + 2]
        }
      }

      return imageData
    }

    function tgaGetImageData32bits (imageData, yStart, yStep, yEnd, xStart, xStep, xEnd, image) {
      var i = 0
      var x
      var y
      var width = header.width

      for (y = yStart; y !== yEnd; y += yStep) {
        for (x = xStart; x !== xEnd; x += xStep, i += 4) {
          imageData[(x + width * y) * 4 + 2] = image[i + 0]
          imageData[(x + width * y) * 4 + 1] = image[i + 1]
          imageData[(x + width * y) * 4 + 0] = image[i + 2]
          imageData[(x + width * y) * 4 + 3] = image[i + 3]
        }
      }

      return imageData
    }

    function tgaGetImageDataGrey8bits (imageData, yStart, yStep, yEnd, xStart, xStep, xEnd, image) {
      var color
      var i = 0
      var x
      var y
      var width = header.width

      for (y = yStart; y !== yEnd; y += yStep) {
        for (x = xStart; x !== xEnd; x += xStep, i++) {
          color = image[i]
          imageData[(x + width * y) * 4 + 0] = color
          imageData[(x + width * y) * 4 + 1] = color
          imageData[(x + width * y) * 4 + 2] = color
          imageData[(x + width * y) * 4 + 3] = 255
        }
      }

      return imageData
    }

    function tgaGetImageDataGrey16bits (imageData, yStart, yStep, yEnd, xStart, xStep, xEnd, image) {
      var i = 0
      var x
      var y
      var width = header.width

      for (y = yStart; y !== yEnd; y += yStep) {
        for (x = xStart; x !== xEnd; x += xStep, i += 2) {
          imageData[(x + width * y) * 4 + 0] = image[i + 0]
          imageData[(x + width * y) * 4 + 1] = image[i + 0]
          imageData[(x + width * y) * 4 + 2] = image[i + 0]
          imageData[(x + width * y) * 4 + 3] = image[i + 1]
        }
      }

      return imageData
    }

    function getTgaRGBA (data, width, height, image, palette) {
      var xStart,
        yStart,
        xStep,
        yStep,
        xEnd,
        yEnd

      switch ((header.flags & TGA_ORIGIN_MASK) >> TGA_ORIGIN_SHIFT) {
      default:
      case TGA_ORIGIN_UL:
        xStart = 0
        xStep = 1
        xEnd = width
        yStart = 0
        yStep = 1
        yEnd = height
        break

      case TGA_ORIGIN_BL:
        xStart = 0
        xStep = 1
        xEnd = width
        yStart = height - 1
        yStep = -1
        yEnd = -1
        break

      case TGA_ORIGIN_UR:
        xStart = width - 1
        xStep = -1
        xEnd = -1
        yStart = 0
        yStep = 1
        yEnd = height
        break

      case TGA_ORIGIN_BR:
        xStart = width - 1
        xStep = -1
        xEnd = -1
        yStart = height - 1
        yStep = -1
        yEnd = -1
        break
      }

      if (useGrey) {
        switch (header.pixel_size) {
        case 8:
          tgaGetImageDataGrey8bits(data, yStart, yStep, yEnd, xStart, xStep, xEnd, image)
          break

        case 16:
          tgaGetImageDataGrey16bits(data, yStart, yStep, yEnd, xStart, xStep, xEnd, image)
          break

        default:
          console.error('TGALoader: Format not supported.')
          break
        }
      } else {
        switch (header.pixel_size) {
        case 8:
          tgaGetImageData8bits(data, yStart, yStep, yEnd, xStart, xStep, xEnd, image, palette)
          break

        case 16:
          tgaGetImageData16bits(data, yStart, yStep, yEnd, xStart, xStep, xEnd, image)
          break

        case 24:
          tgaGetImageData24bits(data, yStart, yStep, yEnd, xStart, xStep, xEnd, image)
          break

        case 32:
          tgaGetImageData32bits(data, yStart, yStep, yEnd, xStart, xStep, xEnd, image)
          break

        default:
          console.error('TGALoader: Format not supported.')
          break
        }
      }

      // Load image data according to specific method
      // var func = 'tgaGetImageData' + (use_grey ? 'Grey' : '') + (header.pixel_size) + 'bits';
      // func(data, y_start, y_step, y_end, x_start, x_step, x_end, width, image, palette );
      return data
    }

    // TGA constants

    var TGA_TYPE_NO_DATA = 0
    var TGA_TYPE_INDEXED = 1
    var TGA_TYPE_RGB = 2
    var TGA_TYPE_GREY = 3
    var TGA_TYPE_RLE_INDEXED = 9
    var TGA_TYPE_RLE_RGB = 10
    var TGA_TYPE_RLE_GREY = 11

    var TGA_ORIGIN_MASK = 0x30
    var TGA_ORIGIN_SHIFT = 0x04
    var TGA_ORIGIN_BL = 0x00
    var TGA_ORIGIN_BR = 0x01
    var TGA_ORIGIN_UL = 0x02
    var TGA_ORIGIN_UR = 0x03

    if (buffer.length < 19) console.error('TGALoader: Not enough data to contain header.')

    var content = new Uint8Array(buffer)
    var offset = 0
    var header = {
      id_length: content[offset++],
      colormap_type: content[offset++],
      image_type: content[offset++],
      colormap_index: content[offset++] | content[offset++] << 8,
      colormap_length: content[offset++] | content[offset++] << 8,
      colormap_size: content[offset++],
      origin: [
        content[offset++] | content[offset++] << 8,
        content[offset++] | content[offset++] << 8,
      ],
      width: content[offset++] | content[offset++] << 8,
      height: content[offset++] | content[offset++] << 8,
      pixel_size: content[offset++],
      flags: content[offset++],
    }

    // check tga if it is valid format

    tgaCheckHeader(header)

    if (header.id_length + offset > buffer.length) {
      console.error('TGALoader: No data.')
    }

    // skip the needn't data

    offset += header.id_length

    // get targa information about RLE compression and palette

    var useRle = false
    var usePal = false
    var useGrey = false

    switch (header.image_type) {
    case TGA_TYPE_RLE_INDEXED:
      useRle = true
      usePal = true
      break

    case TGA_TYPE_INDEXED:
      usePal = true
      break

    case TGA_TYPE_RLE_RGB:
      useRle = true
      break

    case TGA_TYPE_RGB:
      break

    case TGA_TYPE_RLE_GREY:
      useRle = true
      useGrey = true
      break

    case TGA_TYPE_GREY:
      useGrey = true
      break
    }

    //

    var canvas = document.createElement('canvas')
    canvas.width = header.width
    canvas.height = header.height

    var context = canvas.getContext('2d')
    var imageData = context.createImageData(header.width, header.height)

    var result = tgaParse(useRle, usePal, header, offset, content)
    getTgaRGBA(imageData.data, header.width, header.height, result.pixel_data, result.palettes)

    context.putImageData(imageData, 0, 0)

    return canvas
  },

}

export { TGALoader }
