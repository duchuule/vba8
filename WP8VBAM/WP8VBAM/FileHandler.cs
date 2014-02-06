using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using PhoneDirect3DXamlAppInterop.Database;
using Windows.Storage;
using Windows.Storage.Streams;
using System.Windows;
using System.Windows.Media.Imaging;
using System.IO.IsolatedStorage;
using Microsoft.Phone.Shell;
using PhoneDirect3DXamlAppInterop.Resources;
using Windows.Phone.Storage.SharedAccess;
using PhoneDirect3DXamlAppComponent;
using Windows.UI;
using System.IO;

namespace PhoneDirect3DXamlAppInterop
{
    class FileHandler
    {
        public const String ROM_URI_STRING = "rom";
        public const String ROM_DIRECTORY = "roms";
        public const String SAVE_DIRECTORY = "saves";

#if GBC
        public const String DEFAULT_SNAPSHOT_ALT = "Assets/no_snapshot.png";
        public const String DEFAULT_SNAPSHOT = "Assets/no_snapshot_gbc.png";
#else
        public const String DEFAULT_SNAPSHOT_ALT = "Assets/no_snapshot_gbc.png";
        public const String DEFAULT_SNAPSHOT = "Assets/no_snapshot.png";
#endif
        public static DateTime DEFAULT_DATETIME = new DateTime(1989, 02, 22);

        public static String DEFAULT_BACKGROUND_IMAGE = "Assets/GameboyAdvance.jpg";


        public static BitmapImage getBitmapImage(String path, String default_path)
        {

            BitmapImage img = new BitmapImage();

            if (path.Equals(FileHandler.DEFAULT_BACKGROUND_IMAGE))
            {
                Uri uri = new Uri(path, UriKind.Relative);

                img = new BitmapImage(uri);
                return img;
            }

            if (!String.IsNullOrEmpty(path))
            {
                using (var isoStore = IsolatedStorageFile.GetUserStoreForApplication())
                {
                    using (IsolatedStorageFileStream fs = isoStore.OpenFile(path, System.IO.FileMode.Open, System.IO.FileAccess.Read))
                    {
                        img.SetSource(fs);
                    }
                }
            }

            return img;

        }

        public static ROMDBEntry InsertNewDBEntry(string fileName)
        {
            ROMDatabase db = ROMDatabase.Current;

            int index = fileName.LastIndexOf('.');
            int diff = fileName.Length - index;

            string displayName = fileName.Substring(0, fileName.Length - diff);
            ROMDBEntry entry = new ROMDBEntry()
            {
                DisplayName = displayName,
                FileName = fileName,
                LastPlayed = DEFAULT_DATETIME,
                SnapshotURI = DEFAULT_SNAPSHOT
            };
            db.Add(entry);
            return entry;
        }

        public static async Task FindExistingSavestatesForNewROM(ROMDBEntry entry)
        {
            ROMDatabase db = ROMDatabase.Current;
            StorageFolder localFolder = ApplicationData.Current.LocalFolder;
            StorageFolder romFolder = await localFolder.GetFolderAsync(ROM_DIRECTORY);
            StorageFolder saveFolder = await romFolder.GetFolderAsync(SAVE_DIRECTORY);
            IReadOnlyList<StorageFile> saves = await saveFolder.GetFilesAsync();
            // Savestates zuordnen
            foreach (var save in saves)
            {
                String name = save.Name.ToLower();
                String romName = entry.DisplayName.ToLower();
                // Savestate format : (ROMNAME)(SLOT [0-9]).sgm
                if (name.Substring(0, name.Length - 5).Equals(romName) && name.EndsWith(".sgm"))
                {
                    // Savestate gehoert zu ROM
                    String number = save.Name.Substring(save.Name.Length - 5, 1);
                    int slot = 0;
                    if (!int.TryParse(number, out slot))
                    {
                        continue;
                    }
                    SavestateEntry ssEntry = new SavestateEntry()
                    {
                        ROM = entry,
                        Savetime = save.DateCreated.DateTime,
                        Slot = slot,
                        FileName = save.Name
                    };
                    db.Add(ssEntry);
                }
            }
        }

        public static void CaptureSnapshot(byte[] pixeldata, int pitch, string filename)
        {
            Deployment.Current.Dispatcher.BeginInvoke(new Action(() =>
            {
                captureSnapshot(pixeldata, pitch, filename);
            }));
        }

        private static void captureSnapshot(byte[] pixeldata, int pitch, string filename)
        {
            int pixelWidth = pitch / 4;
            int pixelHeight = (int)pixeldata.Length / pitch;
            Bitmap bitmap = new Bitmap(pixelWidth, pixelHeight, 24);
            int x = 0;
            int y = 0;
            for (int i = 0; i < pitch * pixelHeight; i += 4)
            {
                byte r = pixeldata[i];
                byte g = pixeldata[i + 1];
                byte b = pixeldata[i + 2];
                Color c = Color.FromArgb(0xff, r, g, b);
                bitmap.SetColor(x, y, c);
                x++;
                if (x >= pixelWidth)
                {
                    y++;
                    x = 0;
                }
            }

            int index = filename.LastIndexOf('.');
            int diff = filename.Length - 1 - index;

            String snapshotName = filename.Substring(0, filename.Length - diff) + "bmp";
            try
            {
                IsolatedStorageFile iso = IsolatedStorageFile.GetUserStoreForApplication();
                using (IsolatedStorageFileStream fs = new IsolatedStorageFileStream("/Shared/ShellContent/" + snapshotName, System.IO.FileMode.Create, iso))
                {
                    bitmap.Save(fs);

                    fs.Flush(true);
                }
                ROMDatabase db = ROMDatabase.Current;
                ROMDBEntry entry = db.GetROM(filename);
                entry.SnapshotURI = "Shared/ShellContent/" + snapshotName;
                db.CommitChanges();

                UpdateLiveTile();

                UpdateROMTile(filename);
            }
            catch (Exception)
            {
            }

            //WriteableBitmap bitmap = new WriteableBitmap(pitch / 4, (int)pixeldata.Length / (pitch));
            //int x = 0;
            //int y = 0;
            //for (int i = 0; i < pitch * bitmap.PixelHeight; i += 4)
            //{
            //    byte r = pixeldata[i];
            //    byte g = pixeldata[i + 1];
            //    byte b = pixeldata[i + 2];
            //    bitmap.SetPixel(x, y, r, g, b);
            //    x++;
            //    if (x >= bitmap.PixelWidth)
            //    {
            //        y++;
            //        x = 0;
            //    }
            //}

            //int index = filename.LastIndexOf('.');
            //int diff = filename.Length - 1 - index;

            //String snapshotName = filename.Substring(0, filename.Length - diff) + "jpg";
            //try
            //{
            //    IsolatedStorageFile iso = IsolatedStorageFile.GetUserStoreForApplication();
            //    using (IsolatedStorageFileStream fs = new IsolatedStorageFileStream("/Shared/ShellContent/" + snapshotName, System.IO.FileMode.Create, iso))
            //    {
            //        bitmap.SaveJpeg(fs, bitmap.PixelWidth, bitmap.PixelHeight, 0, 90);

            //        fs.Flush(true);
            //    }
            //    ROMDatabase db = ROMDatabase.Current;
            //    ROMDBEntry entry = db.GetROM(filename);
            //    entry.SnapshotURI = "Shared/ShellContent/" + snapshotName;
            //    db.CommitChanges();

            //    UpdateLiveTile();

            //    UpdateROMTile(filename);
            //}
            //catch (Exception)
            //{
            //}
        }

        public static void UpdateLiveTile()
        {
            ROMDatabase db = ROMDatabase.Current;
            ShellTile tile = ShellTile.ActiveTiles.FirstOrDefault();
            CycleTileData data = new CycleTileData();
#if !GBC
            data.Title = AppResources.ApplicationTitle;
#else
            data.Title = AppResources.ApplicationTitle2;
#endif
            IEnumerable<String> snapshots = db.GetRecentSnapshotList();
            List<Uri> uris = new List<Uri>(snapshots.Count());
            if (snapshots.Count() == 0)
            {
                for (int i = 0; i < 9; i++)
                {
#if !GBC
                    uris.Add(new Uri("Assets/Tiles/tileIconLarge.png", UriKind.Relative));
#else
                    uris.Add(new Uri("Assets/Tiles/tileIconLargeGBC.png", UriKind.Relative));
#endif
                }
            }
            else
            {
                int x = 0;
                for (int j = 0; j <= (3 - snapshots.Count()); j++)
                {
                    for (int i = 0; i < 3; i++)
                    {
                        foreach (var snapshot in snapshots)
                        {
                            uris.Add(new Uri("isostore:/" + snapshot, UriKind.Absolute));
                            x++;
                            if (x >= 9)
                            {
                                i = j = 3;
                                break;
                            }
                        }
                    }
                }

            }
            data.CycleImages = uris;

            tile.Update(data);
        }

        public static void DeleteROMTile(string romFileName)
        {
            var tiles = ShellTile.ActiveTiles;
            romFileName = romFileName.ToLower();
            foreach (var tile in tiles)
            {
                int index = tile.NavigationUri.OriginalString.LastIndexOf('=');
                if (index < 0)
                {
                    continue;
                }
                String romName = tile.NavigationUri.OriginalString.Substring(index + 1);
                if (romName.ToLower().Equals(romFileName))
                {
                    tile.Delete();
                }
            }
        }

        public static void CreateROMTile(ROMDBEntry re)
        {
            FlipTileData data = CreateFlipTileData(re);

            ShellTile.Create(new Uri("/MainPage.xaml?" + ROM_URI_STRING + "=" + re.FileName, UriKind.Relative), data, true);
        }

        public static void UpdateROMTile(string romFileName)
        {
            var tiles = ShellTile.ActiveTiles;
            romFileName = romFileName.ToLower();
            foreach (var tile in tiles)
            {
                int index = tile.NavigationUri.OriginalString.LastIndexOf('=');
                if (index < 0)
                {
                    continue;
                }
                String romName = tile.NavigationUri.OriginalString.Substring(index + 1);
                if (romName.ToLower().Equals(romFileName))
                {
                    ROMDatabase db = ROMDatabase.Current;
                    ROMDBEntry entry = db.GetROM(romFileName);
                    if (entry == null)
                    {
                        break;
                    }

                    FlipTileData data = CreateFlipTileData(entry);
                    tile.Update(data);

                    break;
                }
            }
        }

        private static FlipTileData CreateFlipTileData(ROMDBEntry re)
        {
            FlipTileData data = new FlipTileData();
            data.Title = re.DisplayName;
            if (re.SnapshotURI.Equals(FileHandler.DEFAULT_SNAPSHOT) || re.SnapshotURI.Equals(FileHandler.DEFAULT_SNAPSHOT_ALT))
            {
#if !GBC
                data.SmallBackgroundImage = new Uri("Assets/Tiles/FlipCycleTileSmall.png", UriKind.Relative);
                data.BackgroundImage = new Uri("Assets/Tiles/tileIcon.png", UriKind.Relative);
                data.WideBackgroundImage = new Uri("Assets/Tiles/tileIconLarge.png", UriKind.Relative);
#else
                data.SmallBackgroundImage = new Uri("Assets/Tiles/FlipCycleTileSmallGBC.png", UriKind.Relative);
                data.BackgroundImage = new Uri("Assets/Tiles/tileIconGBC.png", UriKind.Relative);
                data.WideBackgroundImage = new Uri("Assets/Tiles/tileIconLargeGBC.png", UriKind.Relative);
#endif
            }
            else
            {
                data.SmallBackgroundImage = new Uri("isostore:/" + re.SnapshotURI, UriKind.Absolute);
                data.BackgroundImage = new Uri("isostore:/" + re.SnapshotURI, UriKind.Absolute);
                data.WideBackgroundImage = new Uri("isostore:/" + re.SnapshotURI, UriKind.Absolute);
            }
            return data;
        }

        public static async Task DeleteROMAsync(ROMDBEntry rom)
        {
            ROMDatabase db = ROMDatabase.Current;
            String fileName = rom.FileName;
            StorageFolder folder = await ApplicationData.Current.LocalFolder.GetFolderAsync(ROM_DIRECTORY);
            StorageFile file = await folder.GetFileAsync(fileName);
            DeleteROMTile(file.Name);
            await file.DeleteAsync(StorageDeleteOption.PermanentDelete);
            db.RemoveROM(file.Name);
            db.CommitChanges();
        }

        public static async Task<LoadROMParameter> GetROMFileToPlayAsync(string fileName)
        {
            StorageFolder localFolder = ApplicationData.Current.LocalFolder;
            StorageFolder romFolder = await localFolder.GetFolderAsync(ROM_DIRECTORY);
            StorageFile romFile = await romFolder.GetFileAsync(fileName);
            LoadROMParameter param = new LoadROMParameter()
            {
                file = romFile,
                folder = romFolder
            };
            return param;
        }

        public static async Task FillDatabaseAsync()
        {
            ROMDatabase db = ROMDatabase.Current;
            StorageFolder localFolder = ApplicationData.Current.LocalFolder;
            StorageFolder romFolder = await localFolder.GetFolderAsync(ROM_DIRECTORY);
            IReadOnlyList<StorageFile> roms = await romFolder.GetFilesAsync();

            foreach (var file in roms)
            {
                ROMDBEntry entry = FileHandler.InsertNewDBEntry(file.Name);
                await FileHandler.FindExistingSavestatesForNewROM(entry);
            }
            db.CommitChanges();
        }

        public static async Task CreateInitialFolderStructure()
        {
            StorageFolder localFolder = ApplicationData.Current.LocalFolder;
            StorageFolder romFolder = await localFolder.CreateFolderAsync(ROM_DIRECTORY, CreationCollisionOption.OpenIfExists);
            StorageFolder saveFolder = await romFolder.CreateFolderAsync(SAVE_DIRECTORY, CreationCollisionOption.OpenIfExists);
        }

        public static void CreateSavestate(int slot, string romFileName)
        {
            Deployment.Current.Dispatcher.BeginInvoke(new Action(() =>
            {
                createSavestate(slot, romFileName);
            }));
        }

        private static void createSavestate(int slot, string romFileName)
        {
            ROMDatabase db = ROMDatabase.Current;

            int index = romFileName.LastIndexOf('.');
            int diff = romFileName.Length - index;

            string saveFileName = romFileName.Substring(0, romFileName.Length - diff);
            saveFileName += slot;
            saveFileName += ".sgm";

            SavestateEntry entry;
            if ((entry = db.GetSavestate(romFileName, slot)) == null)
            {
                ROMDBEntry rom = db.GetROM(romFileName);
                entry = new SavestateEntry()
                {
                    ROM = rom,
                    FileName = saveFileName,
                    ROMFileName = romFileName,
                    Savetime = DateTime.Now,
                    Slot = slot
                };
                db.Add(entry);
            }
            else
            {
                entry.Savetime = DateTime.Now;
            }
            db.CommitChanges();
        }


        //public static async Task RenameROMFile(string oldname, string newname)
        //{
        //    ROMDatabase db = ROMDatabase.Current;


        //    StorageFolder localFolder = ApplicationData.Current.LocalFolder;
        //    StorageFolder romFolder = await localFolder.GetFolderAsync(ROM_DIRECTORY);

        //    StorageFile oldfile = await romFolder.GetFileAsync(oldname);
        //    StorageFile newfile = await romFolder.CreateFileAsync(newname, CreationCollisionOption.ReplaceExisting);

        //    using (var outputStream = await newfile.OpenStreamForWriteAsync())
        //    {
        //        using (var inputStream = await oldfile.OpenStreamForReadAsync())
        //        {
        //            await inputStream.CopyToAsync(outputStream);
        //        }
        //    }

            
        //}



        internal static async Task<ROMDBEntry> ImportRomBySharedID(string importRomID, DependencyObject page)
        {
            ROMDatabase db = ROMDatabase.Current;
            string filename = SharedStorageAccessManager.GetSharedFileName(importRomID).Replace("[1]", "").Replace("%20"," ");


            //set status bar
            var indicator = new ProgressIndicator()
            {
                IsIndeterminate = true,
                IsVisible = true,
                Text = String.Format(AppResources.ImportingProgressText, filename)
            };

            SystemTray.SetProgressIndicator(page, indicator);
            

            StorageFolder localFolder = ApplicationData.Current.LocalFolder;
            StorageFolder romFolder = await localFolder.CreateFolderAsync(ROM_DIRECTORY, CreationCollisionOption.OpenIfExists);
            
            IStorageFile file = await SharedStorageAccessManager.CopySharedFileAsync(romFolder, filename, NameCollisionOption.ReplaceExisting, importRomID);

            ROMDBEntry entry = db.GetROM(file.Name);

            if (entry == null)
            {
                entry = FileHandler.InsertNewDBEntry(file.Name);
                await FileHandler.FindExistingSavestatesForNewROM(entry);
                db.CommitChanges();
            }

            indicator = new ProgressIndicator()
            {
                IsIndeterminate = false,
                IsVisible = true,
#if GBC
                Text = AppResources.ApplicationTitle2
#else
            Text = AppResources.ApplicationTitle
#endif

            };

            SystemTray.SetProgressIndicator(page, indicator);
            MessageBox.Show(String.Format(AppResources.ImportCompleteText, entry.DisplayName));


            return entry;
        }

        internal static async Task DeleteSRAMFile(ROMDBEntry re)
        {
            string sramName = re.FileName.Substring(0, re.FileName.LastIndexOf('.')) + ".sav";

            StorageFolder localFolder = ApplicationData.Current.LocalFolder;
            StorageFolder romFolder = await localFolder.GetFolderAsync(ROM_DIRECTORY);
            StorageFolder saveFolder = await romFolder.GetFolderAsync(SAVE_DIRECTORY);
            try
            {
                IStorageFile file = await saveFolder.GetFileAsync(sramName);
                await file.DeleteAsync();
            }
            catch (Exception) { }
        }

        internal static async Task<List<CheatData>> LoadCheatCodes(ROMDBEntry re)
        {
            List<CheatData> cheats = new List<CheatData>();
            String romFileName = re.FileName;
            int index = romFileName.LastIndexOf('.');
            int diff = romFileName.Length - index;

            string cheatFileName = romFileName.Substring(0, romFileName.Length - diff);
            cheatFileName += ".cht";

            StorageFolder localFolder = ApplicationData.Current.LocalFolder;
            StorageFolder romFolder = await localFolder.GetFolderAsync(ROM_DIRECTORY);
            StorageFolder saveFolder = await romFolder.GetFolderAsync(SAVE_DIRECTORY);

            StorageFile file = null;
            try
            {
                file = await saveFolder.GetFileAsync(cheatFileName);
            }
            catch (System.IO.FileNotFoundException)
            {
                return cheats;
            }
            string codes = null;
            using (var stream = await file.OpenReadAsync())
            {
                using (var readStream = stream.GetInputStreamAt(0L))
                {
                    using (DataReader reader = new DataReader(readStream))
                    {
                        await reader.LoadAsync((uint)stream.Size);
                        codes = reader.ReadString((uint)stream.Size);
                    }
                }
            }
            if (!String.IsNullOrWhiteSpace(codes))
            {
                string[] lines = codes.Split('\n');
                for (int i = 0; i < lines.Length; i += 3)
                {
                    if (lines.Length - i < 3)
                        continue;
                    CheatData data = new CheatData();
                    data.Description = lines[i];
                    data.CheatCode = lines[i + 1];
                    data.Enabled = lines[i + 2].Equals("1");

                    cheats.Add(data);
                }
            }

            return cheats;
        }

        internal static async void SaveCheatCodes(ROMDBEntry re, List<CheatData> cheatCodes)
        {
            String romFileName = re.FileName;
            int index = romFileName.LastIndexOf('.');
            int diff = romFileName.Length - index;

            string cheatFileName = romFileName.Substring(0, romFileName.Length - diff);
            cheatFileName += ".cht";

            StorageFolder localFolder = ApplicationData.Current.LocalFolder;
            StorageFolder romFolder = await localFolder.GetFolderAsync(ROM_DIRECTORY);
            StorageFolder saveFolder = await romFolder.GetFolderAsync(SAVE_DIRECTORY);

            StorageFile file = await saveFolder.CreateFileAsync(cheatFileName, CreationCollisionOption.ReplaceExisting);
            using (IRandomAccessStream stream = await file.OpenAsync(FileAccessMode.ReadWrite))
            {
                using (IOutputStream outStream = stream.GetOutputStreamAt(0L))
                {
                    using (DataWriter writer = new DataWriter(outStream))
                    {
                        writer.UnicodeEncoding = Windows.Storage.Streams.UnicodeEncoding.Utf8;
                        writer.ByteOrder = Windows.Storage.Streams.ByteOrder.LittleEndian;

                        for (int i = 0; i < cheatCodes.Count; i++)
                        {
                            if (i > 0)
                                writer.WriteString("\n");
                            writer.WriteString(cheatCodes[i].Description);
                            writer.WriteString("\n");
                            writer.WriteString(cheatCodes[i].CheatCode);
                            writer.WriteString("\n");
                            writer.WriteString(cheatCodes[i].Enabled ? "1" : "0");
                        }
                        await writer.StoreAsync();
                        await writer.FlushAsync();
                        writer.DetachStream();
                    }
                }
            }
        }

        public static async Task<bool> DeleteSaveState(SavestateEntry entry)
        {
            ROMDatabase db = ROMDatabase.Current;
            if (!db.RemoveSavestateFromDB(entry))
            {
                return false;
            }

            StorageFolder localFolder = ApplicationData.Current.LocalFolder;
            StorageFolder romFolder = await localFolder.GetFolderAsync(ROM_DIRECTORY);
            StorageFolder saveFolder = await romFolder.GetFolderAsync(SAVE_DIRECTORY);

            try
            {
                StorageFile file = await saveFolder.GetFileAsync(entry.FileName);
                await file.DeleteAsync();
            }
            catch (Exception)
            {
                return false;
            }

            return true;
        }
    }
}
