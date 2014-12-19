using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using Windows.Storage;
using Windows.Storage.Streams;
using PhoneDirect3DXamlAppComponent;

namespace PhoneDirect3DXamlAppInterop
{
    class GBAIniParser
    {
        public GBAIniParser()
        { }

        public async Task<IDictionary<String, ROMConfig>> ParseAsync(StorageFile file)
        {
            IDictionary<String, ROMConfig> dict = new Dictionary<String, ROMConfig>();

            String text = null;
            using (var stream = await file.OpenReadAsync())
            using (DataReader reader = new DataReader(stream))
            {
                uint bytesLoaded = await reader.LoadAsync((uint)stream.Size);
                text = reader.ReadString(bytesLoaded);
            }
            if (text == null)
                return dict;

            String[] lines = text.Split('\n');
            for(int i = 0; i < lines.Length; i++)
            {
                String line = lines[i];
                int startBraces = line.IndexOf('[');
                if (startBraces == -1)
                {
                    continue;
                }
                int endBraces = line.IndexOf(']');
                if (endBraces == -1)
                {
                    continue;
                }
                ROMConfig config = new ROMConfig()
                {
                    flashSize = -1,
                    mirroringEnabled = -1,
                    rtcEnabled = -1,
                    saveType = -1
                };
                String romCode = line.Substring(startBraces + 1, endBraces - startBraces - 1);

                // Read the single configuration lines.
                for (i++; i < lines.Length && !String.IsNullOrWhiteSpace(lines[i]); i++)
                {
                    line = lines[i];
                    int equalsIndex = line.IndexOf('=');
                    if (equalsIndex == -1)
                    {
                        continue;
                    }
                    if (equalsIndex + 1 >= line.Length)
                    {
                        continue;
                    }
                    String configName = line.Substring(0, equalsIndex);
                    String configValue = line.Substring(equalsIndex + 1);
                    int value = -1;
                    if (!int.TryParse(configValue, out value))
                    {
                        continue;
                    }
                    switch (configName)
                    {
                        case "rtcEnabled":
                            config.rtcEnabled = value;
                            break;
                        case "flashSize":
                            config.flashSize = value;
                            break;
                        case "saveType":
                            config.saveType = value;
                            break;
                        case "mirroringEnabled":
                            config.mirroringEnabled = value;
                            break;
                    }
                }

                dict.Add(romCode, config);
            }

            return dict;
        }
    }
}
