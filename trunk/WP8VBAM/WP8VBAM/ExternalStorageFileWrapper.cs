using System;
using System.IO;

namespace PhoneDirect3DXamlAppInterop
{
    /// <summary>
    /// Stream wrapper to circumnavigate buggy Stream reading of stream returned by ExternalStorageFile.OpenForReadAsync()
    /// http://stackoverflow.com/questions/15752463/issue-with-reading-file-on-windows-phone-8-sd-card/17355068#17355068
    /// </summary>
    public sealed class NativeFileStreamFixed : Stream
    {
        private Stream _stream; // Underlying stream

        public NativeFileStreamFixed(Stream stream)
        {
            if (stream == null)
                throw new ArgumentNullException("stream");

            _stream = stream;
        }

        public override bool CanRead
        {
            get { return _stream.CanRead; }
        }

        public override bool CanSeek
        {
            get { return _stream.CanSeek; }
        }

        public override bool CanWrite
        {
            get { return _stream.CanWrite; }
        }

        public override void Flush()
        {
            _stream.Flush();
        }

        public override long Length
        {
            get { return _stream.Length; }
        }

        public override long Position
        {
            get
            {
                return _stream.Position;
            }
            set
            {
                _stream.Position = value;
            }
        }

        public override int Read(byte[] buffer, int offset, int count)
        {
            return _stream.Read(buffer, offset, count);
        }

        // NOTE: Will need to be avoided in future versions of Windows Phone once Microsoft has fixed their bug!
        public override long Seek(long offset, SeekOrigin origin)
        {
            ulong uoffset = (ulong)offset;
            ulong fix = ((uoffset & 0xffffffffL) << 32) | ((uoffset & 0xffffffff00000000L) >> 32);
            return _stream.Seek((long)fix, origin);
        }

        public override void SetLength(long value)
        {
            _stream.SetLength(value);
        }

        public override void Write(byte[] buffer, int offset, int count)
        {
            _stream.Write(buffer, offset, count);
        }
    }

}
