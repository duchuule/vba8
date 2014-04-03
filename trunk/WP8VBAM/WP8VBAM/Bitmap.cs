using System;
using System.Collections.Generic;
using System.IO;
using System.IO.IsolatedStorage;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Media;


namespace PhoneDirect3DXamlAppInterop
{
    struct BitmapFileHeader
	{
		public int SizeInBytes;		// 0x0002
        public short Reserved1;		// 0x0006
        public short Reserved2;		// 0x0008
        public int PixelArrayOffset;	// 0x000A

		public BitmapFileHeader(int size, int offset)
		{
            Reserved1 = 0;
            Reserved2 = 0;
            this.SizeInBytes = size;
            this.PixelArrayOffset = offset;
        }
	};

    struct BitmapInfoHeader
	{
        public int HeaderSize;			// 0x000E
        public int PixelWidth;	        // 0x0012
        public int PixelHeight;        // 0x0016
        public short ColorPlanes;		// 0x001A
        public short BPP;				// 0x001C
        public int Compression;		// 0x001E
        public int ImageSize;			// 0x0022
        public int PPMWidth;			// 0x0026
        public int PPMHeight;			// 0x002A
        public int ColorsInPalette;	// 0x002E
        public int UsedPaletteColors;	// 0x0032

        public BitmapInfoHeader(int pixelWidth, int pixelHeight, short bpp)
        {
            HeaderSize = 40;
            ColorPlanes = 1;
            Compression = 0;
            ImageSize = 0;
            PPMWidth = 2000;
            PPMHeight = 2000;
            ColorsInPalette = 0;
            UsedPaletteColors = 0;
            this.PixelWidth = pixelWidth;
            this.PixelHeight = pixelHeight;
            this.BPP = bpp;
        }
	};

	struct BitmapV4Header
	{
        public int HeaderSize;			// 0x000E
        public int PixelWidth;	        // 0x0012
        public int PixelHeight;        // 0x0016
        public short ColorPlanes;		// 0x001A
        public short BPP;				// 0x001C
        public int Compression;		// 0x001E
        public int ImageSize;			// 0x0022
        public int PPMWidth;			// 0x0026
        public int PPMHeight;			// 0x002A
        public int ColorsInPalette;	// 0x002E
        public int UsedPaletteColors;	// 0x0032
        public uint RedMask;
        public uint GreenMask;
        public uint BlueMask;
        public uint AlphaMask;
        public int CSType;
        public byte[] Endpoints;
        public int GammaRed;
        public int GammaGreen;
        public int GammaBlue;

        public BitmapV4Header(int pixelWidth, int pixelHeight, short bpp)
        {
            this.PixelWidth = pixelWidth;
            this.PixelHeight = pixelHeight;
            this.BPP = bpp;
            HeaderSize = 108;
            ColorPlanes = 1;
            Compression = Bitmap.BI_BITFIELDS;
            ImageSize = 0;
            PPMWidth = 2835;
            PPMHeight = 2835;
            ColorsInPalette = 0;
            UsedPaletteColors = 0;
            CSType = 0;
            RedMask = Bitmap.RED_MASK;
            GreenMask = Bitmap.GREEN_MASK;
            BlueMask = Bitmap.BLUE_MASK;
            AlphaMask = Bitmap.ALPHA_MASK;
            GammaRed = 0;
            GammaGreen = 0;
            GammaBlue = 0;
            this.Endpoints = new byte[36];
        }
	};

    public class Bitmap
    {
        public const int BITMAP_HEADER_SIZE = 12 + 108; //(sizeof(BitmapFileHeader) + sizeof(BitmapV4Header) + 2);
        public const int BI_BITFIELDS = 3;
        public const uint ALPHA_MASK = 0xFF000000;
        public const uint RED_MASK = 0x00FF0000;
        public const uint GREEN_MASK = 0x0000FF00;
        public const uint BLUE_MASK = 0x000000FF;

        BitmapFileHeader fileHeader;
        BitmapV4Header v4header;
        byte[] pixels;
        int rowSize;

        public Bitmap(int width, int height, short bpp)
        {
            if (bpp != 24 && bpp != 32)
            {
                throw new ArgumentOutOfRangeException("bpp");
            }
            width = Math.Abs(width);
            height = Math.Abs(height);
            this.rowSize = (int)((bpp * width / 32.0f) * 4.0f);
            this.rowSize += (4 - (this.rowSize % 4)) % 4; // Padding
            int pixelArraySize = rowSize * height;
            int fileSize = BITMAP_HEADER_SIZE + pixelArraySize;
            this.pixels = new byte[pixelArraySize];

            this.fileHeader = new BitmapFileHeader(fileSize, BITMAP_HEADER_SIZE);
            this.v4header = new BitmapV4Header(width, -height, bpp);
        }

        public void SetColor(int x, int y, Color color)
        {
            int width = this.v4header.PixelWidth;
            int height = Math.Abs(this.v4header.PixelHeight);
            if (x < 0 || x >= width)
            {
                throw new ArgumentOutOfRangeException("x");
            }
            if (y < 0 || y >= height)
            {
                throw new ArgumentOutOfRangeException("y");
            }
            int pixelIndex = y * this.rowSize + (x * this.v4header.BPP / 8);
            this.pixels[pixelIndex] = color.B;
            this.pixels[pixelIndex + 1] = color.G;
            this.pixels[pixelIndex + 2] = color.R;
            if (this.v4header.BPP == 32)
            {
                this.pixels[pixelIndex + 3] = color.A;
            }
        }

        public void Save(IsolatedStorageFileStream fs)
        {
            BinaryWriter writer = new BinaryWriter(fs);
            writer.Write((byte)0x42);
            writer.Write((byte)0x4d);

            // File header
            writer.Write(this.fileHeader.SizeInBytes);
            writer.Write(this.fileHeader.Reserved1);
            writer.Write(this.fileHeader.Reserved2);
            writer.Write(this.fileHeader.PixelArrayOffset);

            // V4 Header
            writer.Write(this.v4header.HeaderSize);
            writer.Write(this.v4header.PixelWidth);
            writer.Write(this.v4header.PixelHeight);
            writer.Write(this.v4header.ColorPlanes);
            writer.Write(this.v4header.BPP);
            writer.Write(this.v4header.Compression);
            writer.Write(this.v4header.ImageSize);
            writer.Write(this.v4header.PPMWidth);
            writer.Write(this.v4header.PPMHeight);
            writer.Write(this.v4header.ColorsInPalette);
            writer.Write(this.v4header.UsedPaletteColors);
            writer.Write(this.v4header.RedMask);
            writer.Write(this.v4header.GreenMask);
            writer.Write(this.v4header.BlueMask);
            writer.Write(this.v4header.AlphaMask);
            writer.Write(this.v4header.CSType);
            for (int i = 0; i < 36; i++)
            {
                writer.Write(this.v4header.Endpoints[i]);
            }
            writer.Write(this.v4header.GammaRed);
            writer.Write(this.v4header.GammaGreen);
            writer.Write(this.v4header.GammaBlue);

            // Pixels
            for (int i = 0; i < this.pixels.Length; i++)
            {
                writer.Write(pixels[i]);
            }
        }
    }
}
