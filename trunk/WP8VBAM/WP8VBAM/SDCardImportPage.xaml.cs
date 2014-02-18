/* 
    Copyright (c) 2012 Microsoft Corporation.  All rights reserved.
    Use of this sample source code is subject to the terms of the Microsoft license 
    agreement under which you licensed this sample source code and is provided AS-IS.
    If you did not accept the terms of the license agreement, you are not authorized 
    to use this sample source code.  For the terms of the license, please see the 
    license agreement between you and Microsoft.
  
    To see all Code Samples for Windows Phone, visit http://go.microsoft.com/fwlink/?LinkID=219604 
  
*/
using Microsoft.Phone.Controls;
using Microsoft.Phone.Storage;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.IO;
using System.Linq;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Navigation;
using Microsoft.Phone.Shell;
using System.Threading.Tasks;



using PhoneDirect3DXamlAppInterop.Resources;
using PhoneDirect3DXamlAppInterop.Database;
using Windows.Storage;
using Windows.Storage.Streams;



namespace PhoneDirect3DXamlAppInterop
{

    public partial class SDCardImportPage : PhoneApplicationPage
    {


        // A collection of routes (.GPX files) for binding to the ListBox.
        public ObservableCollection<ExternalStorageFile> Routes { get; set; }
        List<List<SDCardListItem>> skydriveStack;
        int downloadsInProgress = 0;
        bool initialPageLoaded = false;


        ExternalStorageDevice sdCard = null;

        // Constructor
        public SDCardImportPage()
        {
            InitializeComponent();

            //create ad control
            if (App.HasAds)
            {
                AdControl adControl = new AdControl();
                ((Grid)(LayoutRoot.Children[0])).Children.Add(adControl);
                adControl.SetValue(Grid.RowProperty, 2);
            }



#if GBC
            SystemTray.GetProgressIndicator(this).Text = AppResources.ApplicationTitle2;
#endif





            // Initialize the collection for routes.
            Routes = new ObservableCollection<ExternalStorageFile>();

            this.skydriveStack = new List<List<SDCardListItem>>();
            this.skydriveStack.Add(new List<SDCardListItem>());
            

            // Enable data binding to the page itself.
            this.DataContext = this;
        }


        protected override async void OnNavigatedTo(NavigationEventArgs e)
        {
            // Connect to the current SD card.
            sdCard = (await ExternalStorage.GetExternalStorageDevicesAsync()).FirstOrDefault();

            // If the SD card is present, add GPX files to the Routes collection.
            if (sdCard != null)
            {

                this.skydriveStack[0].Add(new SDCardListItem()
                {
                    Name = "Root",
                    isFolder = true,
                    ThisFolder = sdCard.RootFolder,
                    Parent = null
                });


                this.OpenFolder(sdCard.RootFolder);

            }
            else
            {
                // No SD card is present.
                MessageBox.Show("The SD card is missing. Insert an SD card try again.");
            }

            base.OnNavigatedTo(e);
        }


        async void OpenFolder(ExternalStorageFolder folderToOpen)
        {
            try
            {
                //new list
                List<SDCardListItem> listItems = new List<SDCardListItem>();


                // Get all folders on root
                List<ExternalStorageFolder> listFolders = (await folderToOpen.GetFoldersAsync()).ToList();
                foreach (ExternalStorageFolder folder in listFolders)
                {

                    SDCardListItem item = new SDCardListItem()
                    {
                        isFolder = true,
                        Name = folder.Name,
                        Parent = folderToOpen,
                        ThisFolder = folder
                    };
                    listItems.Add(item);
                }

                List<ExternalStorageFile> listFiles = (await folderToOpen.GetFilesAsync()).ToList();
                foreach (ExternalStorageFile file in listFiles)
                {

                    SDCardListItem item = new SDCardListItem()
                    {
                        isFolder = false,
                        Name = file.Name,
                        Parent = folderToOpen,
                        ThisFile = file
                    };
                    if (item.ThisFile.Path.ToLower().EndsWith(".gb") || item.ThisFile.Path.ToLower().EndsWith(".gbc") || item.ThisFile.Path.ToLower().EndsWith(".gba"))
                        item.Type = SkyDriveItemType.ROM;
                    else if (item.ThisFile.Path.ToLower().EndsWith(".sav"))
                        item.Type = SkyDriveItemType.SRAM;
                    else if (item.ThisFile.Path.ToLower().EndsWith(".sgm"))
                        item.Type = SkyDriveItemType.Savestate;
                    listItems.Add(item);
                }

               
                    

                this.skydriveStack.Add(listItems);
                this.skydriveList.ItemsSource = listItems;
            }
            catch (Exception)
            {
                MessageBox.Show("Error reading SD Card content");
            }
        }


        async void skydriveList_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            SDCardListItem item = this.skydriveList.SelectedItem as SDCardListItem;
            if (item == null)
                return;

            ROMDatabase db = ROMDatabase.Current;

            if (item.isFolder)
            {
                this.skydriveList.ItemsSource = null;
                this.currentFolderBox.Text = item.Name;
                this.OpenFolder(item.ThisFolder);

            }
            else if (item.Type == SkyDriveItemType.ROM)
            {
                await this.ImportROM(item.ThisFile);  

            }
            else if (item.Type == SkyDriveItemType.Savestate || item.Type == SkyDriveItemType.SRAM)
            {
                //check to make sure there is a rom with matching name
                ROMDBEntry entry = null;

                if (item.Type == SkyDriveItemType.Savestate)  //save state
                    entry = db.GetROMFromSavestateName(item.Name);
                else if (item.Type == SkyDriveItemType.SRAM) //sram
                    entry = db.GetROMFromSRAMName(item.Name);

                if (entry == null) //no matching file name
                {
                    MessageBox.Show(AppResources.NoMatchingNameText, AppResources.ErrorCaption, MessageBoxButton.OK);
                    return;
                }

                //check to make sure format is right
                if (item.Type == SkyDriveItemType.Savestate)  //save state
                {
                    string slot = item.Name.Substring(item.Name.Length - 5, 1);
                    int parsedSlot = 0;
                    if (!int.TryParse(slot, out parsedSlot))
                    {
                        MessageBox.Show(AppResources.ImportSavestateInvalidFormat, AppResources.ErrorCaption, MessageBoxButton.OK);
                        return;
                    }
                }
                
                // Download
                await this.ImportSave(item);


            }
 
        }

        private async Task ImportSave(SDCardListItem item)
        {

            var indicator = SystemTray.GetProgressIndicator(this);
            indicator.IsIndeterminate = true;
            indicator.Text = String.Format(AppResources.ImportingProgressText, item.Name);

            StorageFolder folder = ApplicationData.Current.LocalFolder;
            StorageFolder romFolder = await folder.CreateFolderAsync(FileHandler.ROM_DIRECTORY, CreationCollisionOption.OpenIfExists);
            StorageFolder saveFolder = await romFolder.CreateFolderAsync(FileHandler.SAVE_DIRECTORY, CreationCollisionOption.OpenIfExists);

            String path = romFolder.Path;
            String savePath = saveFolder.Path;

            ROMDatabase db = ROMDatabase.Current;

            try
            {
                Stream s = await item.ThisFile.OpenForReadAsync();


                byte[] tmpBuf = new byte[s.Length];
                StorageFile destinationFile = null;

                ROMDBEntry entry = null;

                if (item.Type == SkyDriveItemType.SRAM)
                {
                    entry = db.GetROMFromSRAMName(item.Name);
                    if (entry != null)
                        destinationFile = await saveFolder.CreateFileAsync(Path.GetFileNameWithoutExtension(entry.FileName) + ".sav", CreationCollisionOption.ReplaceExisting);

                    else
                        destinationFile = await saveFolder.CreateFileAsync(item.Name, CreationCollisionOption.ReplaceExisting);
                }
                else if (item.Type == SkyDriveItemType.Savestate)
                {
                    entry = db.GetROMFromSavestateName(item.Name);


                    if (entry != null)
                        destinationFile = await saveFolder.CreateFileAsync(Path.GetFileNameWithoutExtension(entry.FileName) + item.Name.Substring(item.Name.Length - 5), CreationCollisionOption.ReplaceExisting);
                    else
                        destinationFile = await saveFolder.CreateFileAsync(item.Name, CreationCollisionOption.ReplaceExisting);
                }

                using (IRandomAccessStream destStream = await destinationFile.OpenAsync(FileAccessMode.ReadWrite))
                using (DataWriter writer = new DataWriter(destStream))
                {
                    while (s.Read(tmpBuf, 0, tmpBuf.Length) != 0)
                    {
                        writer.WriteBytes(tmpBuf);
                    }
                    await writer.StoreAsync();
                    await writer.FlushAsync();
                    writer.DetachStream();
                }
                s.Close();
                item.Downloading = false;



                if (item.Type == SkyDriveItemType.Savestate)
                {
                    String number = item.Name.Substring(item.Name.Length - 5, 1);
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
                            FileName = item.Name
                        };
                        db.Add(ssEntry);
                        db.CommitChanges();

                    }
                }

                MessageBox.Show(String.Format(AppResources.DownloadCompleteText, item.Name));
            }
            catch (Exception ex)
            {
               
                MessageBox.Show(String.Format(AppResources.DownloadErrorText, item.Name, ex.Message), AppResources.ErrorCaption, MessageBoxButton.OK);
            }
#if GBC
            indicator.Text = AppResources.ApplicationTitle2;
#else
            indicator.Text = AppResources.ApplicationTitle;
#endif
            indicator.IsIndeterminate = false;
        }


        protected override void OnBackKeyPress(System.ComponentModel.CancelEventArgs e)
        {

            if (this.downloadsInProgress > 0)
            {
                MessageBox.Show(AppResources.ImportWaitComplete);
                e.Cancel = true;
                return;
            }

            if (this.skydriveStack.Count <= 2)
            {
                return;
            }

            try
            {
                this.skydriveStack.RemoveAt(this.skydriveStack.Count - 1);
                this.skydriveList.ItemsSource = this.skydriveStack.Last();

                string parentName = this.skydriveStack[this.skydriveStack.Count - 2].Where(x => x.isFolder).First(item =>
                {
                    return item.ThisFolder.Path.Equals(this.skydriveStack[this.skydriveStack.Count - 1][0].Parent.Path);
                }).Name;

                this.currentFolderBox.Text = parentName;

                e.Cancel = true;
            }
            catch (Exception) { }

            base.OnBackKeyPress(e);
        }



        private async Task ImportROM(ExternalStorageFile file)
        {
            var indicator = SystemTray.GetProgressIndicator(this);
            indicator.IsIndeterminate = true;
            indicator.Text = String.Format(AppResources.ImportingProgressText, file.Name);

            try
            {
                ROMDatabase db = ROMDatabase.Current;
                StorageFolder folder = ApplicationData.Current.LocalFolder;
                StorageFolder romFolder = await folder.CreateFolderAsync("roms", CreationCollisionOption.OpenIfExists);

                Stream s = await file.OpenForReadAsync();

                byte[] tmpBuf = new byte[s.Length];
                StorageFile destinationFile = await romFolder.CreateFileAsync(file.Name, CreationCollisionOption.ReplaceExisting);

                using (IRandomAccessStream destStream = await destinationFile.OpenAsync(FileAccessMode.ReadWrite))
                using (DataWriter writer = new DataWriter(destStream))
                {
                    while (s.Read(tmpBuf, 0, tmpBuf.Length) != 0)
                    {
                        writer.WriteBytes(tmpBuf);
                    }
                    await writer.StoreAsync();
                    await writer.FlushAsync();
                    writer.DetachStream();
                }
                s.Close();

                

                var romEntry = db.GetROM(file.Name);
                bool fileExisted = false;
                if (romEntry != null)
                    fileExisted = true;

                if (!fileExisted)
                {
                    var entry = FileHandler.InsertNewDBEntry(destinationFile.Name);
                    await FileHandler.FindExistingSavestatesForNewROM(entry);
                    db.CommitChanges();
                }

                MessageBox.Show(String.Format(AppResources.ImportCompleteText, destinationFile.Name));
            }
            catch (Exception ex)
            {
                //MessageBox.Show(String.Format(AppResources.DownloadErrorText, file.Name, ex.Message), AppResources.ErrorCaption, MessageBoxButton.OK);
            }

#if GBC
            indicator.Text = AppResources.ApplicationTitle2;
#else
            indicator.Text = AppResources.ApplicationTitle;
#endif
            indicator.IsIndeterminate = false;
        }


    } //end class


    public class SDCardListItem
    {
        public String Name { get; set; }
        public bool isFolder { get; set; } //true if folder, false if file
        public ExternalStorageFile ThisFile { get; set; } //only 1 of ThisFile or ThisFolder will be set
        public ExternalStorageFolder ThisFolder { get; set; }
        public ExternalStorageFolder Parent { get; set; }
        public int FolderChildrenCount { get; set; }
        public bool Downloading { get; set; }
        public SkyDriveItemType Type { get; set; }
        

    }
}
