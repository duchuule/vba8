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
using System.Windows.Media;
using System.IO;
using System.Windows.Resources;
using DucLe.Imaging;

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
        public static String CUSTOM_TILE_FILENAME = "myCustomTile1.png";


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
            ROMDatabase.Current.Add(entry);
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

                    if (slot != 9) //dont include auto save
                    {
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

                CreateOrUpdateSecondaryTile(false);

                UpdateROMTile(filename);
            }
            catch (Exception)
            {
            }

        }

        public static void UpdateLiveTile()
        {
            ROMDatabase db = ROMDatabase.Current;
            ShellTile tile = ShellTile.ActiveTiles.FirstOrDefault();


            FlipTileData data = new FlipTileData();
#if !GBC
            data.Title = AppResources.ApplicationTitle;
#else
            data.Title = AppResources.ApplicationTitle2;
#endif

            //get last snapshot
            IEnumerable<String> lastSnapshots = db.GetRecentSnapshotList();

            if (App.metroSettings.UseAccentColor ||  lastSnapshots.Count() == 0 )  //create see through tile
            {
#if !GBC
                data.SmallBackgroundImage = new Uri("Assets/Tiles/FlipCycleTileSmall.png", UriKind.Relative);
                data.BackgroundImage = new Uri("Assets/Tiles/FlipCycleTileMedium.png", UriKind.Relative);
                data.WideBackgroundImage = new Uri("Assets/Tiles/FlipCycleTileLarge.png", UriKind.Relative);
#else
                data.SmallBackgroundImage = new Uri("Assets/Tiles/FlipCycleTileSmallGBC.png", UriKind.Relative);
                data.BackgroundImage = new Uri("Assets/Tiles/FlipCycleTileMediumGBC.png", UriKind.Relative);
                data.WideBackgroundImage = new Uri("Assets/Tiles/FlipCycleTileLargeGBC.png", UriKind.Relative);
#endif
                tile.Update(data);
            }
            else  //create opaque tile
            {
#if !GBC
                data.SmallBackgroundImage = new Uri("Assets/Tiles/FlipCycleTileSmallFilled.png", UriKind.Relative);
#else
                data.SmallBackgroundImage = new Uri("Assets/Tiles/FlipCycleTileSmallFilledGBC.png", UriKind.Relative);
#endif


                data.BackgroundImage = new Uri("isostore:/" + lastSnapshots.ElementAt(0), UriKind.Absolute);
                data.WideBackgroundImage = new Uri("isostore:/" + lastSnapshots.ElementAt(0), UriKind.Absolute);

                if (lastSnapshots.Count() >= 2)
                {
                    data.BackBackgroundImage = new Uri("isostore:/" + lastSnapshots.ElementAt(1), UriKind.Absolute);
                    data.WideBackBackgroundImage = new Uri("isostore:/" + lastSnapshots.ElementAt(1), UriKind.Absolute);
                }

                tile.Update(data);
            }

        }


        public static void CreateOrUpdateSecondaryTile(bool forceCreate)
        {
            
            

            CycleTileData data = new CycleTileData();


#if !GBC
            data.Title = AppResources.ApplicationTitle;
#else
            data.Title = AppResources.ApplicationTitle2;
#endif
            IEnumerable<String> snapshots = ROMDatabase.Current.GetRecentSnapshotList();
            List<Uri> uris = new List<Uri>();
            if (snapshots.Count() == 0)
            {
#if !GBC
                uris.Add(new Uri("Assets/Tiles/FlipCycleTileLarge.png", UriKind.Relative));
#else
                    uris.Add(new Uri("Assets/Tiles/FlipCycleTileLargeGBC.png", UriKind.Relative));
#endif
            }
            else
            {
                foreach (var snapshot in snapshots)
                {
                    uris.Add(new Uri("isostore:/" + snapshot, UriKind.Absolute));
                }

            }
            data.CycleImages = uris;



#if GBC
            data.SmallBackgroundImage = new Uri("Assets/Tiles/FlipCycleTileSmallGBC.png", UriKind.Relative);
#else
            data.SmallBackgroundImage = new Uri("Assets/Tiles/FlipCycleTileSmall.png", UriKind.Relative);
#endif
            //find the tile to update
            bool secondaryTileAlreadyExists = false;

            
            foreach (var tile in ShellTile.ActiveTiles)
            {
                //make sure if it is not the default
                if (tile.NavigationUri.OriginalString == "/")
                    continue;

                //rom tile has '=' sign, secondary does not have it
                int index = tile.NavigationUri.OriginalString.LastIndexOf('=');
                if (index >= 0)
                {
                    continue; //this is a rom tile
                }

                //if arrive at this point, it means secondary tile already exists
                secondaryTileAlreadyExists = true;

                tile.Update(data);

                if (forceCreate)
                    MessageBox.Show(AppResources.TileAlreadyPinnedText);

                //no need to find more tile
                break;
            }



            if (secondaryTileAlreadyExists == false && forceCreate)
                ShellTile.Create(new Uri("/MainPage.xaml", UriKind.Relative), data, true);
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
                data.BackgroundImage = new Uri("Assets/Tiles/FlipCycleTileMedium.png", UriKind.Relative);
                data.WideBackgroundImage = new Uri("Assets/Tiles/FlipCycleTileLarge.png", UriKind.Relative);
#else
                data.SmallBackgroundImage = new Uri("Assets/Tiles/FlipCycleTileSmallGBC.png", UriKind.Relative);
                data.BackgroundImage = new Uri("Assets/Tiles/FlipCycleTileMediumGBC.png", UriKind.Relative);
                data.WideBackgroundImage = new Uri("Assets/Tiles/FlipCycleTileLargeGBC.png", UriKind.Relative);
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

            String fileName = rom.FileName;
            StorageFolder folder = await ApplicationData.Current.LocalFolder.GetFolderAsync(ROM_DIRECTORY);
            StorageFile file = await folder.GetFileAsync(fileName);
            DeleteROMTile(file.Name);
            await file.DeleteAsync(StorageDeleteOption.PermanentDelete);

            

            ROMDatabase.Current.RemoveROM(file.Name);
            ROMDatabase.Current.CommitChanges();
            
        }

        public static async Task<LoadROMParameter> GetROMFileToPlayAsync(string fileName)
        {
            StorageFolder localFolder = ApplicationData.Current.LocalFolder;
            StorageFolder romFolder = await localFolder.GetFolderAsync(ROM_DIRECTORY);
            StorageFile romFile = await romFolder.GetFileAsync(fileName);
            LoadROMParameter param = new LoadROMParameter()
            {
                file = romFile,
                folder = romFolder,
                RomFileName = fileName
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



        internal static async Task<ROMDBEntry> ImportRomBySharedID(string fileID, string desiredName, DependencyObject page)
        {
            //note: desiredName can be different from the file name obtained from fileID
            ROMDatabase db = ROMDatabase.Current;


            //set status bar
            var indicator = SystemTray.GetProgressIndicator(page);
            indicator.IsIndeterminate = true;
            indicator.Text = String.Format(AppResources.ImportingProgressText, desiredName);

            

            StorageFolder localFolder = ApplicationData.Current.LocalFolder;
            StorageFolder romFolder = await localFolder.CreateFolderAsync(ROM_DIRECTORY, CreationCollisionOption.OpenIfExists);
            
            IStorageFile file = await SharedStorageAccessManager.CopySharedFileAsync(romFolder, desiredName, NameCollisionOption.ReplaceExisting, fileID);

            
            ROMDBEntry entry = db.GetROM(file.Name);

            if (entry == null)
            {
                entry = FileHandler.InsertNewDBEntry(file.Name);
                await FileHandler.FindExistingSavestatesForNewROM(entry);
                db.CommitChanges();
            }

            //update voice command list
            await MainPage.UpdateGameListForVoiceCommand();

#if GBC
            indicator.Text = AppResources.ApplicationTitle2;
#else
            indicator.Text = AppResources.ApplicationTitle;
#endif
            indicator.IsIndeterminate = false;

            MessageBox.Show(String.Format(AppResources.ImportCompleteText, entry.DisplayName));


            return entry;
        }


        internal static async Task ImportSaveBySharedID(string fileID, string actualName, DependencyObject page)
        {
            //note:  the file name obtained from fileID can be different from actualName if the file is obtained through cloudsix
            ROMDatabase db = ROMDatabase.Current;


            //check to make sure there is a rom with matching name
            ROMDBEntry entry = null;
            string extension = Path.GetExtension(actualName).ToLower();

            if (extension == ".sgm") 
                entry = db.GetROMFromSavestateName(actualName);
            else if (extension == ".sav")
                entry = db.GetROMFromSRAMName(actualName);

            if (entry == null) //no matching file name
            {
                MessageBox.Show(AppResources.NoMatchingNameText, AppResources.ErrorCaption, MessageBoxButton.OK);
                return;
            }

            //check to make sure format is right
            if (extension == ".sgm")
            {
                string slot = actualName.Substring(actualName.Length - 5, 1);
                int parsedSlot = 0;
                if (!int.TryParse(slot, out parsedSlot))
                {
                    MessageBox.Show(AppResources.ImportSavestateInvalidFormat, AppResources.ErrorCaption, MessageBoxButton.OK);
                    return;
                }
            }



            //set status bar
            var indicator = SystemTray.GetProgressIndicator(page);
            indicator.IsIndeterminate = true;
            indicator.Text = String.Format(AppResources.ImportingProgressText, actualName);



            StorageFolder localFolder = ApplicationData.Current.LocalFolder;
            StorageFolder romFolder = await localFolder.CreateFolderAsync(ROM_DIRECTORY, CreationCollisionOption.OpenIfExists);
            StorageFolder saveFolder = await romFolder.CreateFolderAsync(FileHandler.SAVE_DIRECTORY, CreationCollisionOption.OpenIfExists);


            //if arrive here, entry cannot be null, we can copy the file
            IStorageFile file = null;
            if (extension == ".sgm") 
                file = await SharedStorageAccessManager.CopySharedFileAsync(saveFolder, Path.GetFileNameWithoutExtension(entry.FileName) + actualName.Substring(actualName.Length - 5), NameCollisionOption.ReplaceExisting, fileID);
            else if (extension == ".sav")
            {
                file = await SharedStorageAccessManager.CopySharedFileAsync(saveFolder, Path.GetFileNameWithoutExtension(entry.FileName) + ".sav", NameCollisionOption.ReplaceExisting, fileID);
                entry.SuspendAutoLoadLastState = true;
            }

            //update database
            if (extension == ".sgm")
            {
                String number = actualName.Substring(actualName.Length - 5, 1);
                int slot = int.Parse(number);

                if (entry != null) //NULL = do nothing
                {
                    SavestateEntry saveentry = db.SavestateEntryExisting(entry.FileName, slot);
                    if (saveentry != null)
                    {
                        //delete entry
                        db.RemoveSavestateFromDB(saveentry);

                    }
                    SavestateEntry ssEntry = new SavestateEntry()
                    {
                        ROM = entry,
                        Savetime = DateTime.Now,
                        Slot = slot,
                        FileName = actualName
                    };
                    db.Add(ssEntry);
                    db.CommitChanges();

                }
            }




#if GBC
            indicator.Text = AppResources.ApplicationTitle2;
#else
            indicator.Text = AppResources.ApplicationTitle;
#endif
            indicator.IsIndeterminate = false;

            MessageBox.Show(String.Format(AppResources.ImportCompleteText, entry.DisplayName));


            return;
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
            try
            {
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
                catch (Exception)
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
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message);
            }

            return cheats;
        }

        public static async Task SaveCheatCodes(ROMDBEntry re, List<CheatData> cheatCodes)
        {
            try
            {
                String romFileName = re.FileName;
                int index = romFileName.LastIndexOf('.');
                int diff = romFileName.Length - index;

                string cheatFileName = romFileName.Substring(0, romFileName.Length - diff);
                cheatFileName += ".cht";

                StorageFolder localFolder = ApplicationData.Current.LocalFolder;
                StorageFolder romFolder = await localFolder.GetFolderAsync(ROM_DIRECTORY);
                StorageFolder saveFolder = await romFolder.GetFolderAsync(SAVE_DIRECTORY);

               
                StorageFile file = await saveFolder.CreateFileAsync("cheattmp.cht", CreationCollisionOption.ReplaceExisting);
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

                //rename the temp file to the offical file
                await file.RenameAsync(cheatFileName, NameCollisionOption.ReplaceExisting);

            }
            catch (Exception ex)
            {
                MessageBox.Show("Save cheat code error: " + ex.Message);
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

        private static Uri CreateAndSavePNGToIsolatedStorage(Uri logo, Color tileColor)
        {
            // Obtain the virtual store for the application.
            using (IsolatedStorageFile myStore = IsolatedStorageFile.GetUserStoreForApplication())
            {
                using (IsolatedStorageFileStream fileStream = new IsolatedStorageFileStream("Shared/ShellContent/" + CUSTOM_TILE_FILENAME, FileMode.Create, myStore))
                {
                    using (Stream pngStream = RenderAsPNGStream(logo, tileColor))
                    {
                        pngStream.CopyTo(fileStream);
                    }
                }
            }

            return new Uri("isostore:/" + "Shared/ShellContent/" + CUSTOM_TILE_FILENAME, UriKind.Absolute);
        }

        private static Stream RenderAsPNGStream(Uri logo, Color tileColor)
        {
            try
            {
                StreamResourceInfo info;

                info = Application.GetResourceStream(logo);

                // create source bitmap for Image control (image is assumed to be alread 173x173)
                WriteableBitmap wbmp3 = new WriteableBitmap(1, 1);
                try
                {
                    wbmp3.SetSource(info.Stream);
                }
                catch
                {
                }

                WriteableBitmap wb = WriteableBitmapEx.CreateTile(wbmp3, tileColor);



                EditableImage edit = new EditableImage(wb.PixelWidth, wb.PixelHeight);

                for (int y = 0; y < wb.PixelHeight; ++y)
                {
                    for (int x = 0; x < wb.PixelWidth; ++x)
                    {
                        try
                        {
                            byte[] rgba = ControlToPng.ExtractRGBAfromPremultipliedARGB(wb.Pixels[wb.PixelWidth * y + x]);
                            edit.SetPixel(x, y, rgba[0], rgba[1], rgba[2], rgba[3]);
                        }
                        catch (Exception ex)
                        { }
                    }
                }

                return edit.GetStream();

            }
            catch (Exception ex)
            {
                return null;
            }
        }
    }
}
