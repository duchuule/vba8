﻿using System;
using System.Collections.Generic;
using System.Linq;
using System.Net;
using System.IO.IsolatedStorage;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Navigation;
using Microsoft.Phone.Controls;
using Microsoft.Phone.Shell;
using Microsoft.Live;
using Microsoft.Live.Controls;
using PhoneDirect3DXamlAppInterop.Resources;
using Windows.Storage;
using System.Threading.Tasks;
using PhoneDirect3DXamlAppComponent;
using PhoneDirect3DXamlAppInterop.Database;
using System.Windows.Media;
using Microsoft.Phone.Tasks;
using Windows.Phone.Storage.SharedAccess;
using System.IO;

//"C:\Program Files (x86)\Microsoft SDKs\Windows Phone\v8.0\Tools\IsolatedStorageExplorerTool\ISETool.exe" ts xd 0a409e81-ab14-47f3-bd4e-2f57bb5bae9a "D:\Duc\Documents\Visual Studio 2012\Projects\WP8VBA8\trunk"

namespace PhoneDirect3DXamlAppInterop
{
    class ROMEntry
    {
        public String Name { get; set; }
    }

    class LoadROMParameter
    {
        public StorageFile file;
        public StorageFolder folder;
    }

    public partial class MainPage : PhoneApplicationPage
    {
        private ApplicationBarIconButton resumeButton;
        private LiveConnectSession session;
        private ROMDatabase db;
        private Task createFolderTask, copyDemoTask, initTask;

        public MainPage()
        {
            InitializeComponent();

#if BETA
            //get license request
            HttpWebRequest request = (HttpWebRequest)WebRequest.Create("http://personal.utulsa.edu/~duc-le/VBA8betalicense.dat");
            request.Method = "POST";
            request.ContentType = "text/plain";
            request.BeginGetResponse(ResponseCallback, request);


#endif
            //create ad control
            if (App.HasAds)
            {
                AdControl adControl = new AdControl();
                LayoutRoot.Children.Add(adControl);
                adControl.SetValue(Grid.RowProperty, 1);
            }

            this.InitAppBar();

            this.initTask = this.Initialize();

            this.Loaded += MainPage_Loaded;

#if GBC
            SystemTray.GetProgressIndicator(this).Text = AppResources.ApplicationTitle2;
            //this.mainPanorama.Title = AppResources.ApplicationTitle2;
#endif
        }

        async void MainPage_Loaded(object sender, RoutedEventArgs e)
        {
            await this.initTask;

            try
            {
                String romFileName = NavigationContext.QueryString[FileHandler.ROM_URI_STRING];
                NavigationContext.QueryString.Remove(FileHandler.ROM_URI_STRING);

                ROMDBEntry entry = this.db.GetROM(romFileName);
                await this.StartROM(entry);
            }
            catch (KeyNotFoundException)
            { }
            catch (Exception)
            {
                MessageBox.Show(AppResources.TileOpenError, AppResources.ErrorCaption, MessageBoxButton.OK);
            }

            try
            {
                String importRomID = NavigationContext.QueryString["fileToken"];
                NavigationContext.QueryString.Remove("fileToken");

                int romCount = this.db.GetNumberOfROMs();
                if (!App.IsTrial || romCount < 2)
                {

                    //import file
                    ROMDBEntry entry = await FileHandler.ImportRomBySharedID(importRomID, this);
                    //await this.StartROM(entry);

                    

                }
                else
                {
                    this.ShowBuyImportDialog();
                }
            }
            catch (KeyNotFoundException)
            { }
            catch (Exception)
            {
                MessageBox.Show(AppResources.FileAssociationError, AppResources.ErrorCaption, MessageBoxButton.OK);
            }
        }

#if BETA
        void ResponseCallback(IAsyncResult result)
        {
            try
            {
                HttpWebRequest request = result.AsyncState as HttpWebRequest;

                HttpWebResponse response = request.EndGetResponse(result) as HttpWebResponse;

                using (StreamReader sr = new StreamReader(response.GetResponseStream()))
                {
                    string responseText = sr.ReadToEnd();

                    if (int.Parse(responseText) == 0)
                    {
                        System.Windows.Deployment.Current.Dispatcher.BeginInvoke(() =>
                        {
                            MessageBox.Show("Trial expired. Exiting now.");

                            Application.Current.Terminate();

                        });


                    }
                }
            }
            catch (Exception ex)
            {
                //System.Windows.Deployment.Current.Dispatcher.BeginInvoke(() =>
                //{
                //    MessageBox.Show("Cannot get trial license. Exiting now.");

                //    Application.Current.Terminate();

                //});
            }
        }
#endif
        private async Task Initialize()
        {
            createFolderTask = FileHandler.CreateInitialFolderStructure();
            copyDemoTask = this.CopyDemoROM();

            await createFolderTask;
            await copyDemoTask;

            this.db = ROMDatabase.Current;
            if (db.Initialize())
            {
                await FileHandler.FillDatabaseAsync();
            }
            this.db.Commit += () =>
            {
                this.RefreshROMList();
            };
            this.RefreshROMList();

            await this.ParseIniFile();
        }

        protected override async void OnNavigatedTo(NavigationEventArgs e)
        {
            //await this.createFolderTask;
            //await this.copyDemoTask;
            await this.initTask;

            this.LoadInitialSettings();

            this.RefreshROMList();

            this.resumeButton.IsEnabled = EmulatorPage.ROMLoaded;


            base.OnNavigatedTo(e);
        }


        void btnSignin_SessionChanged(object sender, Microsoft.Live.Controls.LiveConnectSessionChangedEventArgs e)
        {
            if (e.Status == LiveConnectSessionStatus.Connected)
            {
                this.session = e.Session;
                //this.statusLabel.Text = AppResources.StatusSignedIn;
                this.gotoImportButton.IsEnabled = true;
                this.gotoBackupButton.IsEnabled = true;
                this.gotoRestoreButton.IsEnabled = true;
            }
            else
            {
                this.gotoImportButton.IsEnabled = false;
                this.gotoBackupButton.IsEnabled = false;
                this.gotoRestoreButton.IsEnabled = false;
                //this.statusLabel.Text = AppResources.StatusNotSignedIn;
                session = null;

                //if (e.Error != null)
                //{
                //    MessageBox.Show(String.Format(AppResources.SkyDriveError, e.Error.Message), AppResources.ErrorCaption, MessageBoxButton.OK);
                //    //statusLabel.Text = e.Error.ToString();
                //}
            }
        }

        private void gotoImportButton_Click_1(object sender, RoutedEventArgs e)
        {
            int romCount = this.db.GetNumberOfROMs();
            if (!App.IsTrial || romCount < 2)
            {
                if (session != null)
                {
                    PhoneApplicationService.Current.State["parameter"] = this.session;
                    this.NavigationService.Navigate(new Uri("/SkyDriveImportPage.xaml", UriKind.Relative));
                }
                else
                {
                    MessageBox.Show(AppResources.NotSignedInError, AppResources.ErrorCaption, MessageBoxButton.OK);
                }
            }
            else
            {
                this.ShowBuyImportDialog();
            }
        }







        protected override void OnNavigatedFrom(NavigationEventArgs e)
        {
            FileHandler.UpdateLiveTile();

            base.OnNavigatedFrom(e);
        }

        private async Task ParseIniFile()
        {
            StorageFile file = await StorageFile.GetFileFromPathAsync("Assets/vba-over.ini");

            GBAIniParser parser = new GBAIniParser();
            EmulatorSettings.Current.ROMConfigurations = await parser.ParseAsync(file);
        }

        private async Task CopyDemoROM()
        {
            IsolatedStorageSettings isoSettings = IsolatedStorageSettings.ApplicationSettings;
            if (!isoSettings.Contains("DEMOCOPIED"))
            {
                StorageFolder localFolder = ApplicationData.Current.LocalFolder;
                StorageFolder romFolder = await localFolder.CreateFolderAsync("roms", CreationCollisionOption.OpenIfExists);
#if !GBC
                StorageFile file = await StorageFile.GetFileFromPathAsync("Assets/Bunny Advance (Demo).gba");
#else
                StorageFile file = await StorageFile.GetFileFromPathAsync("Assets/Pong.gb");
#endif
                await file.CopyAsync(romFolder);

                isoSettings["DEMOCOPIED"] = true;
                isoSettings.Save();
            }
        }

        private void LoadInitialSettings()
        {
            EmulatorSettings settings = EmulatorSettings.Current;
            if (!settings.Initialized)
            {
                IsolatedStorageSettings isoSettings = IsolatedStorageSettings.ApplicationSettings;
                settings.Initialized = true;

                if (!isoSettings.Contains(SettingsPage.EnableSoundKey))
                {
                    isoSettings[SettingsPage.EnableSoundKey] = true;
                }
                if (!isoSettings.Contains(SettingsPage.VControllerPosKey))
                {
                    isoSettings[SettingsPage.VControllerPosKey] = false;
                }
                if (!isoSettings.Contains(SettingsPage.LowFreqModeKey))
                {
                    isoSettings[SettingsPage.LowFreqModeKey] = false;
                }
                //if (!isoSettings.Contains(SettingsPage.LowFreqModeMeasuredKey))
                //{
                //    isoSettings[SettingsPage.LowFreqModeMeasuredKey] = false;
                //}
                if (!isoSettings.Contains(SettingsPage.OrientationKey))
                {
                    isoSettings[SettingsPage.OrientationKey] = 0;
                }
                if (!isoSettings.Contains(SettingsPage.ControllerScaleKey))
                {
                    isoSettings[SettingsPage.ControllerScaleKey] = 100;
                }
                if (!isoSettings.Contains(SettingsPage.ButtonScaleKey))
                {
                    isoSettings[SettingsPage.ButtonScaleKey] = 100;
                }
                if (!isoSettings.Contains(SettingsPage.OpacityKey))
                {
                    isoSettings[SettingsPage.OpacityKey] = 30;
                }
                if (!isoSettings.Contains(SettingsPage.AspectKey))
                {
                    isoSettings[SettingsPage.AspectKey] = AspectRatioMode.Original;
                }
                if (!isoSettings.Contains(SettingsPage.SkipFramesKey))
                {
                    isoSettings[SettingsPage.SkipFramesKey] = 0;
                }
                if (!isoSettings.Contains(SettingsPage.ImageScalingKey))
                {
                    isoSettings[SettingsPage.ImageScalingKey] = 100;
                }
                if (!isoSettings.Contains(SettingsPage.TurboFrameSkipKey))
                {
#if GBC
                    // Must be divisible by two for GBC
                    isoSettings[SettingsPage.TurboFrameSkipKey] = 4;
#else
                    isoSettings[SettingsPage.TurboFrameSkipKey] = 4;
#endif
                }
                if (!isoSettings.Contains(SettingsPage.SyncAudioKey))
                {
                    isoSettings[SettingsPage.SyncAudioKey] = true;
                }
                if (!isoSettings.Contains(SettingsPage.PowerSaverKey))
                {
                    isoSettings[SettingsPage.PowerSaverKey] = 0;
                }
                if (!isoSettings.Contains(SettingsPage.DPadStyleKey))
                {
                    isoSettings[SettingsPage.DPadStyleKey] = 1;
                }
                if (!isoSettings.Contains(SettingsPage.DeadzoneKey))
                {
                    isoSettings[SettingsPage.DeadzoneKey] = 10.0f;
                }
                if (!isoSettings.Contains(SettingsPage.CameraAssignKey))
                {
                    isoSettings[SettingsPage.CameraAssignKey] = 0;
                }
                if (!isoSettings.Contains(SettingsPage.ConfirmationKey))
                {
                    isoSettings[SettingsPage.ConfirmationKey] = false;
                }
                if (!isoSettings.Contains(SettingsPage.ConfirmationLoadKey))
                {
                    isoSettings[SettingsPage.ConfirmationLoadKey] = false;
                }
                if (!isoSettings.Contains(SettingsPage.AutoIncKey))
                {
                    isoSettings[SettingsPage.AutoIncKey] = false;
                }
                if (!isoSettings.Contains(SettingsPage.SelectLastState))
                {
                    isoSettings[SettingsPage.SelectLastState] = true;
                }
                if (!isoSettings.Contains(SettingsPage.RestoreCheatKey))
                {
                    isoSettings[SettingsPage.RestoreCheatKey] = false;
                }
                if (!isoSettings.Contains(SettingsPage.CreateManualSnapshotKey))
                {
                    isoSettings[SettingsPage.CreateManualSnapshotKey] = false;
                }
                if (!isoSettings.Contains(SettingsPage.UseMogaControllerKey))
                {
#if BETA
                    isoSettings[SettingsPage.UseMogaControllerKey] = true;
#else
                    isoSettings[SettingsPage.UseMogaControllerKey] = false;
#endif
                }

                //get default controller position
                int[] cpos = CustomizeControllerPage.GetDefaultControllerPosition();
                

                //set default controller position
                if (!isoSettings.Contains(SettingsPage.PadCenterXPKey))
                {
                    isoSettings[SettingsPage.PadCenterXPKey] = cpos[0];
                }
                if (!isoSettings.Contains(SettingsPage.PadCenterYPKey))
                {
                    isoSettings[SettingsPage.PadCenterYPKey] = cpos[1];
                }
                if (!isoSettings.Contains(SettingsPage.ALeftPKey))
                {
                    isoSettings[SettingsPage.ALeftPKey] = cpos[2];
                }
                if (!isoSettings.Contains(SettingsPage.ATopPKey))
                {
                    isoSettings[SettingsPage.ATopPKey] = cpos[3];
                }
                if (!isoSettings.Contains(SettingsPage.BLeftPKey))
                {
                    isoSettings[SettingsPage.BLeftPKey] = cpos[4];
                }
                if (!isoSettings.Contains(SettingsPage.BTopPKey))
                {
                    isoSettings[SettingsPage.BTopPKey] = cpos[5];
                }
                if (!isoSettings.Contains(SettingsPage.StartLeftPKey))
                {
                    isoSettings[SettingsPage.StartLeftPKey] = cpos[6];
                }
                if (!isoSettings.Contains(SettingsPage.StartTopPKey))
                {
                    isoSettings[SettingsPage.StartTopPKey] = cpos[7];
                }
                if (!isoSettings.Contains(SettingsPage.SelectRightPKey))
                {
                    isoSettings[SettingsPage.SelectRightPKey] = cpos[8];
                }
                if (!isoSettings.Contains(SettingsPage.SelectTopPKey))
                {
                    isoSettings[SettingsPage.SelectTopPKey] = cpos[9];
                }
                if (!isoSettings.Contains(SettingsPage.LLeftPKey))
                {
                    isoSettings[SettingsPage.LLeftPKey] = cpos[10];
                }
                if (!isoSettings.Contains(SettingsPage.LTopPKey))
                {
                    isoSettings[SettingsPage.LTopPKey] = cpos[11];
                }
                if (!isoSettings.Contains(SettingsPage.RRightPKey))
                {
                    isoSettings[SettingsPage.RRightPKey] = cpos[12];
                }
                if (!isoSettings.Contains(SettingsPage.RTopPKey))
                {
                    isoSettings[SettingsPage.RTopPKey] = cpos[13];
                }
                if (!isoSettings.Contains(SettingsPage.PadCenterXLKey))
                {
                    isoSettings[SettingsPage.PadCenterXLKey] = cpos[14];
                }
                if (!isoSettings.Contains(SettingsPage.PadCenterYLKey))
                {
                    isoSettings[SettingsPage.PadCenterYLKey] = cpos[15];
                }
                if (!isoSettings.Contains(SettingsPage.ALeftLKey))
                {
                    isoSettings[SettingsPage.ALeftLKey] = cpos[16];
                }
                if (!isoSettings.Contains(SettingsPage.ATopLKey))
                {
                    isoSettings[SettingsPage.ATopLKey] = cpos[17];
                }
                if (!isoSettings.Contains(SettingsPage.BLeftLKey))
                {
                    isoSettings[SettingsPage.BLeftLKey] = cpos[18];
                }
                if (!isoSettings.Contains(SettingsPage.BTopLKey))
                {
                    isoSettings[SettingsPage.BTopLKey] = cpos[19];
                }
                if (!isoSettings.Contains(SettingsPage.StartLeftLKey))
                {
                    isoSettings[SettingsPage.StartLeftLKey] = cpos[20];
                }
                if (!isoSettings.Contains(SettingsPage.StartTopLKey))
                {
                    isoSettings[SettingsPage.StartTopLKey] = cpos[21];
                }
                if (!isoSettings.Contains(SettingsPage.SelectRightLKey))
                {
                    isoSettings[SettingsPage.SelectRightLKey] = cpos[22];
                }
                if (!isoSettings.Contains(SettingsPage.SelectTopLKey))
                {
                    isoSettings[SettingsPage.SelectTopLKey] = cpos[23];
                }
                if (!isoSettings.Contains(SettingsPage.LLeftLKey))
                {
                    isoSettings[SettingsPage.LLeftLKey] = cpos[24];
                }
                if (!isoSettings.Contains(SettingsPage.LTopLKey))
                {
                    isoSettings[SettingsPage.LTopLKey] = cpos[25];
                }
                if (!isoSettings.Contains(SettingsPage.RRightLKey))
                {
                    isoSettings[SettingsPage.RRightLKey] = cpos[26];
                }
                if (!isoSettings.Contains(SettingsPage.RTopLKey))
                {
                    isoSettings[SettingsPage.RTopLKey] = cpos[27];
                }
                isoSettings.Save();

                settings.LowFrequencyMode = (bool)isoSettings[SettingsPage.LowFreqModeKey];
                settings.SoundEnabled = (bool)isoSettings[SettingsPage.EnableSoundKey];
                //settings.VirtualControllerOnTop = (bool)isoSettings[SettingsPage.VControllerPosKey];
                //settings.LowFrequencyModeMeasured = (bool)isoSettings[SettingsPage.LowFreqModeMeasuredKey];
                settings.Orientation = (int)isoSettings[SettingsPage.OrientationKey];
                settings.ControllerScale = (int)isoSettings[SettingsPage.ControllerScaleKey];
                settings.ButtonScale = (int)isoSettings[SettingsPage.ButtonScaleKey];
                settings.ControllerOpacity = (int)isoSettings[SettingsPage.OpacityKey];
                settings.AspectRatio = (AspectRatioMode)isoSettings[SettingsPage.AspectKey];
                settings.FrameSkip = (int)isoSettings[SettingsPage.SkipFramesKey];
                settings.ImageScaling = (int)isoSettings[SettingsPage.ImageScalingKey];
                settings.TurboFrameSkip = (int)isoSettings[SettingsPage.TurboFrameSkipKey];
                settings.SynchronizeAudio = (bool)isoSettings[SettingsPage.SyncAudioKey];
                settings.PowerFrameSkip = (int)isoSettings[SettingsPage.PowerSaverKey];
                settings.DPadStyle = (int)isoSettings[SettingsPage.DPadStyleKey];
                settings.Deadzone = (float)isoSettings[SettingsPage.DeadzoneKey];
                settings.CameraButtonAssignment = (int)isoSettings[SettingsPage.CameraAssignKey];
                settings.AutoIncrementSavestates = (bool)isoSettings[SettingsPage.AutoIncKey];
                settings.HideConfirmationDialogs = (bool)isoSettings[SettingsPage.ConfirmationKey];
                settings.HideLoadConfirmationDialogs = (bool)isoSettings[SettingsPage.ConfirmationLoadKey];
                settings.SelectLastState = (bool)isoSettings[SettingsPage.SelectLastState];
                settings.RestoreOldCheatValues = (bool)isoSettings[SettingsPage.RestoreCheatKey];
                settings.ManualSnapshots = (bool)isoSettings[SettingsPage.CreateManualSnapshotKey];
                settings.UseMogaController = (bool)isoSettings[SettingsPage.UseMogaControllerKey];

                settings.PadCenterXP = (int)isoSettings[SettingsPage.PadCenterXPKey];
                settings.PadCenterYP = (int)isoSettings[SettingsPage.PadCenterYPKey];
                settings.ALeftP = (int)isoSettings[SettingsPage.ALeftPKey];
                settings.ATopP = (int)isoSettings[SettingsPage.ATopPKey];
                settings.BLeftP = (int)isoSettings[SettingsPage.BLeftPKey];
                settings.BTopP = (int)isoSettings[SettingsPage.BTopPKey];
                settings.StartLeftP = (int)isoSettings[SettingsPage.StartLeftPKey];
                settings.StartTopP = (int)isoSettings[SettingsPage.StartTopPKey];
                settings.SelectRightP = (int)isoSettings[SettingsPage.SelectRightPKey];
                settings.SelectTopP = (int)isoSettings[SettingsPage.SelectTopPKey];
                settings.LLeftP = (int)isoSettings[SettingsPage.LLeftPKey];
                settings.LTopP = (int)isoSettings[SettingsPage.LTopPKey];
                settings.RRightP = (int)isoSettings[SettingsPage.RRightPKey];
                settings.RTopP = (int)isoSettings[SettingsPage.RTopPKey];


                settings.PadCenterXL = (int)isoSettings[SettingsPage.PadCenterXLKey];
                settings.PadCenterYL = (int)isoSettings[SettingsPage.PadCenterYLKey];
                settings.ALeftL = (int)isoSettings[SettingsPage.ALeftLKey];
                settings.ATopL = (int)isoSettings[SettingsPage.ATopLKey];
                settings.BLeftL = (int)isoSettings[SettingsPage.BLeftLKey];
                settings.BTopL = (int)isoSettings[SettingsPage.BTopLKey];
                settings.StartLeftL = (int)isoSettings[SettingsPage.StartLeftLKey];
                settings.StartTopL = (int)isoSettings[SettingsPage.StartTopLKey];
                settings.SelectRightL = (int)isoSettings[SettingsPage.SelectRightLKey];
                settings.SelectTopL = (int)isoSettings[SettingsPage.SelectTopLKey];
                settings.LLeftL = (int)isoSettings[SettingsPage.LLeftLKey];
                settings.LTopL = (int)isoSettings[SettingsPage.LTopLKey];
                settings.RRightL = (int)isoSettings[SettingsPage.RRightLKey];
                settings.RTopL = (int)isoSettings[SettingsPage.RTopLKey];


                settings.SettingsChanged = this.SettingsChangedDelegate;
            }
        }

        

        private void SettingsChangedDelegate()
        {
            EmulatorSettings settings = EmulatorSettings.Current;
            IsolatedStorageSettings isoSettings = IsolatedStorageSettings.ApplicationSettings;

            isoSettings[SettingsPage.EnableSoundKey] = settings.SoundEnabled;
            //isoSettings[SettingsPage.VControllerPosKey] = settings.VirtualControllerOnTop;
            isoSettings[SettingsPage.LowFreqModeKey] = settings.LowFrequencyMode;
            //isoSettings[SettingsPage.LowFreqModeMeasuredKey] = settings.LowFrequencyModeMeasured;
            isoSettings[SettingsPage.OrientationKey] = settings.Orientation;
            isoSettings[SettingsPage.ControllerScaleKey] = settings.ControllerScale;
            isoSettings[SettingsPage.ButtonScaleKey] = settings.ButtonScale;
            isoSettings[SettingsPage.OpacityKey] = settings.ControllerOpacity;
            isoSettings[SettingsPage.SkipFramesKey] = settings.FrameSkip;
            isoSettings[SettingsPage.AspectKey] = settings.AspectRatio;
            isoSettings[SettingsPage.ImageScalingKey] = settings.ImageScaling;
            isoSettings[SettingsPage.TurboFrameSkipKey] = settings.TurboFrameSkip;
            isoSettings[SettingsPage.SyncAudioKey] = settings.SynchronizeAudio;
            isoSettings[SettingsPage.PowerSaverKey] = settings.PowerFrameSkip;
            isoSettings[SettingsPage.DPadStyleKey] = settings.DPadStyle;
            isoSettings[SettingsPage.DeadzoneKey] = settings.Deadzone;
            isoSettings[SettingsPage.CameraAssignKey] = settings.CameraButtonAssignment;
            isoSettings[SettingsPage.ConfirmationKey] = settings.HideConfirmationDialogs;
            isoSettings[SettingsPage.AutoIncKey] = settings.AutoIncrementSavestates;
            isoSettings[SettingsPage.ConfirmationLoadKey] = settings.HideLoadConfirmationDialogs;
            isoSettings[SettingsPage.SelectLastState] = settings.SelectLastState;
            isoSettings[SettingsPage.RestoreCheatKey] = settings.RestoreOldCheatValues;
            isoSettings[SettingsPage.CreateManualSnapshotKey] = settings.ManualSnapshots;
            isoSettings[SettingsPage.UseMogaControllerKey] = settings.UseMogaController;
            isoSettings.Save();
        }

        private void RefreshROMList()
        {
            //StorageFolder localFolder = ApplicationData.Current.LocalFolder;
            //StorageFolder romFolder = await localFolder.CreateFolderAsync(ROM_DIRECTORY, CreationCollisionOption.OpenIfExists);
            //IReadOnlyList<StorageFile> roms = await romFolder.GetFilesAsync();
            //IList<ROMEntry> romNames = new List<ROMEntry>(roms.Count);
            //foreach (var file in roms)
            //{
            //    romNames.Add(new ROMEntry() { Name = file.Name } );
            //}
            this.romList.ItemsSource = this.db.GetROMList();// romNames;
            this.recentList.ItemsSource = this.db.GetRecentlyPlayed();
        }

        private void romList_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            this.StartROMFromList(this.romList);
        }

        private void recentList_SelectionChanged_1(object sender, SelectionChangedEventArgs e)
        {
            this.StartROMFromList(this.recentList);
        }

        private async void StartROMFromList(ListBox list)
        {
            if (list.SelectedItem == null)
                return;

            ROMDBEntry entry = (ROMDBEntry)list.SelectedItem;
            list.SelectedItem = null;

            await StartROM(entry);
        }

        private async Task StartROM(ROMDBEntry entry)
        {
            LoadROMParameter param = await FileHandler.GetROMFileToPlayAsync(entry.FileName);

            entry.LastPlayed = DateTime.Now;
            this.db.CommitChanges();

            PhoneApplicationService.Current.State["parameter"] = param;
            this.NavigationService.Navigate(new Uri("/EmulatorPage.xaml", UriKind.Relative));
        }

        private void InitAppBar()
        {
            ApplicationBar = new ApplicationBar();
            ApplicationBar.IsMenuEnabled = true;

            var helpButton = new ApplicationBarMenuItem(AppResources.HelpButtonText);
            helpButton.Click += helpButton_Click;
            ApplicationBar.MenuItems.Add(helpButton);

            var aboutItem = new ApplicationBarMenuItem(AppResources.aboutText);
            aboutItem.Click += aboutItem_Click;
            ApplicationBar.MenuItems.Add(aboutItem);

            resumeButton = new ApplicationBarIconButton(new Uri("/Assets/Icons/transport.play.png", UriKind.Relative))
            {
                Text = AppResources.ResumeButtonText,
                IsEnabled = false
            };
            resumeButton.Click += resumeButton_Click;
            ApplicationBar.Buttons.Add(resumeButton);

            var settingsButton = new ApplicationBarIconButton(new Uri("/Assets/Icons/feature.settings.png", UriKind.Relative))
            {
                Text = AppResources.SettingsButtonText
            };
            settingsButton.Click += settingsButton_Click;
            ApplicationBar.Buttons.Add(settingsButton);


            var purchaseButton = new ApplicationBarIconButton(new Uri("/Assets/Icons/marketplace.png", UriKind.Relative))
            {
                Text = AppResources.PurchaseText
            };
            purchaseButton.Click += purchaseButton_Click;
            ApplicationBar.Buttons.Add(purchaseButton);

            var reviewButton = new ApplicationBarIconButton(new Uri("/Assets/Icons/social.like.png", UriKind.Relative))
            {
                Text = AppResources.ReviewText
            };
            reviewButton.Click += reviewButton_Click;
            ApplicationBar.Buttons.Add(reviewButton);
        }

        private void purchaseButton_Click(object sender, EventArgs e)
        {
            this.NavigationService.Navigate(new Uri("/PurchasePage.xaml", UriKind.Relative));
        }

        private void reviewButton_Click(object sender, EventArgs e)
        {
            MarketplaceReviewTask marketplaceReviewTask = new MarketplaceReviewTask();

            marketplaceReviewTask.Show();
        }

        void helpButton_Click(object sender, EventArgs e)
        {
            this.NavigationService.Navigate(new Uri("/HelpPage.xaml", UriKind.Relative));
        }

        void aboutItem_Click(object sender, EventArgs e)
        {
            this.NavigationService.Navigate(new Uri("/AboutPage.xaml", UriKind.Relative));
        }

        void resumeButton_Click(object sender, EventArgs e)
        {
            this.NavigationService.Navigate(new Uri("/EmulatorPage.xaml", UriKind.Relative));
            this.romList.SelectedItem = null;
        }

        void settingsButton_Click(object sender, EventArgs e)
        {
            this.NavigationService.Navigate(new Uri("/SettingsPage.xaml", UriKind.Relative));
        }

        void importButton_Click(object sender, EventArgs e)
        {
            this.NavigationService.Navigate(new Uri("/ImportPage.xaml", UriKind.Relative));
        }

        private void gotoBackupButton_Click_1(object sender, RoutedEventArgs e)
        {
            if (!App.IsTrial)
            {
                if (session != null)
                {
                    PhoneApplicationService.Current.State["parameter"] = this.session;
                    this.NavigationService.Navigate(new Uri("/BackupPage.xaml", UriKind.Relative));
                }
                else
                {
                    MessageBox.Show(AppResources.NotSignedInError, AppResources.ErrorCaption, MessageBoxButton.OK);
                }
            }
            else
            {
                ShowBuyDialog();
            }
        }

        private void gotoRestoreButton_Click_1(object sender, RoutedEventArgs e)
        {
            if (!App.IsTrial)
            {
                if (session != null)
                {
                    PhoneApplicationService.Current.State["parameter"] = this.session;
                    this.NavigationService.Navigate(new Uri("/RestorePage.xaml", UriKind.Relative));
                }
                else
                {
                    MessageBox.Show(AppResources.NotSignedInError, AppResources.ErrorCaption, MessageBoxButton.OK);
                }
            }
            else
            {
                ShowBuyDialog();
            }
        }

        void ShowBuyDialog()
        {
            ShowDialog(AppResources.BuyNowText);
        }

        void ShowBuyImportDialog()
        {
            ShowDialog(AppResources.BuyNowImportText);
        }

        private static void ShowDialog(String text)
        {
            var result = MessageBox.Show(text, AppResources.InfoCaption, MessageBoxButton.OKCancel);
            if (result == MessageBoxResult.OK)
            {
                MarketplaceDetailTask marketplaceDetailTask = new MarketplaceDetailTask();
                marketplaceDetailTask.ContentType = MarketplaceContentType.Applications;
#if !GBC
                marketplaceDetailTask.ContentIdentifier = "4e3142c4-b99c-4075-bedc-b10a3086327d";
#else
                marketplaceDetailTask.ContentIdentifier = "be33ce3e-e519-4d2c-b30e-83347601ed57";                    
#endif
                marketplaceDetailTask.Show();
            }
        }

        private void StackPanel_Hold(object sender, System.Windows.Input.GestureEventArgs e)
        {
            ROMDBEntry entry = (sender as StackPanel).DataContext as ROMDBEntry;
            PhoneApplicationService.Current.State["parameter"] = entry;

            this.NavigationService.Navigate(new Uri("/ContextPage.xaml", UriKind.Relative));
        }

        private void ImportSD_Click(object sender, RoutedEventArgs e)
        {
            this.NavigationService.Navigate(new Uri("/SDCardImportPage.xaml", UriKind.Relative));
        }



        private void TextBlock_Tap_1(object sender, System.Windows.Input.GestureEventArgs e)
        {
            WebBrowserTask wbtask = new WebBrowserTask();
            wbtask.Uri = new Uri("http://www.youtube.com/watch?v=YfqzZhcr__o");
            wbtask.Show();
        }

        private void TextBlock_Tap(object sender, System.Windows.Input.GestureEventArgs e)
        {
            WebBrowserTask wbtask = new WebBrowserTask();
            wbtask.Uri = new Uri("http://www.youtube.com/watch?v=3WopTRM4ets");
            wbtask.Show();
        }

        private void TextBlock_Tap_2(object sender, System.Windows.Input.GestureEventArgs e)
        {
            WebBrowserTask wbtask = new WebBrowserTask();
            wbtask.Uri = new Uri("http://forums.wpcentral.com/windows-phone-apps/252987-trio-nintendo-emulators-new-tutorial-using-cheat-codes.html");
            wbtask.Show();
        }

        private void contactBlock_Tap_1(object sender, System.Windows.Input.GestureEventArgs e)
        {
            EmailComposeTask emailcomposer = new EmailComposeTask();

            emailcomposer.To = AppResources.AboutContact;
            emailcomposer.Subject = "bug report or feature suggestion";
            emailcomposer.Show();
        }

        private void TextBlock_Tap_3(object sender, System.Windows.Input.GestureEventArgs e)
        {
            this.NavigationService.Navigate(new Uri("/HelpPage.xaml", UriKind.Relative));
        }

        private void TextBlock_Tap_4(object sender, System.Windows.Input.GestureEventArgs e)
        {
            this.NavigationService.Navigate(new Uri("/HelpPage.xaml?index=1", UriKind.Relative));
        }

        private void TextBlock_Tap_5(object sender, System.Windows.Input.GestureEventArgs e)
        {
            this.NavigationService.Navigate(new Uri("/HelpPage.xaml?index=2", UriKind.Relative));
        }


    }
}