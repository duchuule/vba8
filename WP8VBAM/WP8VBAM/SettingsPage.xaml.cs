using System;
using System.Collections.Generic;
using System.Linq;
using System.Net;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Navigation;
using Microsoft.Phone.Controls;
using Microsoft.Phone.Shell;
using PhoneDirect3DXamlAppInterop.Resources;
using PhoneDirect3DXamlAppComponent;
using Microsoft.Phone.Tasks;
using System.Windows.Media.Imaging;
using System.Windows.Controls.Primitives;
using System.IO.IsolatedStorage;
using System.Windows.Media;
using System.Security.Cryptography;

namespace PhoneDirect3DXamlAppInterop
{
    public partial class SettingsPage : PhoneApplicationPage
    {
        public Popup popupWindow = null;

        private String[] frameskiplist = { AppResources.FrameSkipAutoSetting, "0", "1", "2", "3", "4", "5", "6", "7", "8", "9" };
        private String[] frameskiplist2 = { "0", "1", "2", "3", "4", "5", "6", "7", "8", "9" };
        private String[] aspectRatioList = { AppResources.AspectRatioOriginalSetting, AppResources.AspectRatioStretchSetting, AppResources.AspectRatioOneSetting, AppResources.AspectRatio4to3Setting, AppResources.AspectRatio5to4Setting };
        private String[] orientationList = { AppResources.OrientationBoth, AppResources.OrientationLandscape, AppResources.OrientationPortrait };

        public static bool shouldUpdateBackgroud = false;

        public const String VControllerPosKey = "VirtualControllerOnTop";
        public const String EnableSoundKey = "EnableSound";
        public const String LowFreqModeKey = "LowFrequencyModeNew";
        public const String OrientationKey = "Orientation";
        public const String ControllerScaleKey = "ControllerScale";
        public const String ButtonScaleKey = "ButtonScale";
        public const String StretchKey = "FullscreenStretch";
        public const String OpacityKey = "ControllerOpacity";
        public const String AspectKey = "AspectRatioModeKey";
        public const String SkipFramesKey = "SkipFramesKey";
        public const String ImageScalingKey = "ImageScalingKey";
        public const String TurboFrameSkipKey = "TurboSkipFramesKey";
        public const String SyncAudioKey = "SynchronizeAudioKey";
        public const String PowerSaverKey = "PowerSaveSkipKey";
        public const String DPadStyleKey = "DPadStyleKey";
        public const String DeadzoneKey = "DeadzoneKey";
        public const String CameraAssignKey = "CameraAssignmentKey";
        public const String ConfirmationKey = "ConfirmationKey";
        public const String ConfirmationLoadKey = "ConfirmationLoadKey";
        public const String AutoIncKey = "AutoIncKey";
        public const String SelectLastState = "SelectLastStateKey";
        public const String RestoreCheatKey = "RestoreCheatKey";
        public const String CreateManualSnapshotKey = "ManualSnapshotKey";
        public const String UseMogaControllerKey = "UseMogaControllerKey";
        public const String UseColorButtonKey = "UseColorButtonKey";
        public const String BgcolorRKey = "BgcolorRKey";
        public const String BgcolorGKey = "BgcolorGKey";
        public const String BgcolorBKey = "BgcolorBKey";
        public const String AutoSaveLoadKey = "AutSaveLoadKey";
        

        public const String PadCenterXPKey = "PadCenterXPKey";
        public const String PadCenterYPKey = "PadCenterYPKey";
        public const String ALeftPKey = "ALeftPKey";
        public const String ATopPKey = "ATopPKey";
        public const String BLeftPKey = "BLeftPKey";
        public const String BTopPKey = "BTopPKey";
        public const String StartLeftPKey = "StartLeftPKey";
        public const String StartTopPKey = "StartTopPKey";
        public const String SelectRightPKey = "SelectRightPKey";
        public const String SelectTopPKey = "SelectTopPKey";
        public const String LLeftPKey = "LLeftPKey";
        public const String LTopPKey = "LTopPKey";
        public const String RRightPKey = "RRightPKey";
        public const String RTopPKey = "RTopPKey";


        public const String PadCenterXLKey = "PadCenterXLKey";
        public const String PadCenterYLKey = "PadCenterYLKey";
        public const String ALeftLKey = "ALeftLKey";
        public const String ATopLKey = "ATopLKey";
        public const String BLeftLKey = "BLeftLKey";
        public const String BTopLKey = "BTopLKey";
        public const String StartLeftLKey = "StartLeftLKey";
        public const String StartTopLKey = "StartTopLKey";
        public const String SelectRightLKey = "SelectRightLKey";
        public const String SelectTopLKey = "SelectTopLKey";
        public const String LLeftLKey = "LLeftLKey";
        public const String LTopLKey = "LTopLKey";
        public const String RRightLKey = "RRightLKey";
        public const String RTopLKey = "RTopLKey";

        public const String MogaAKey = "MogaAKey";
        public const String MogaBKey = "MogaBKey";
        public const String MogaXKey = "MogaXKey";
        public const String MogaYKey = "MogaYKey";
        public const String MogaL1Key = "MogaL1Key";
        public const String MogaL2Key = "MogaL2Key";
        public const String MogaR1Key = "MogaR1Key";
        public const String MogaR2Key = "MogaR2Key";
        public const String MogaLeftJoystickKey = "MogaLeftJoystickKey";
        public const String MogaRightJoystickKey = "MogaRightJoystickKey";

        bool initdone = false;

        

      

        public SettingsPage()
        {
            InitializeComponent();

#if GBC
            SystemTray.GetProgressIndicator(this).Text = AppResources.ApplicationTitle2;
#endif
            //create ad control
            if (App.HasAds)
            {
                AdControl adControl = new AdControl();
                LayoutRoot.Children.Add(adControl);
                adControl.SetValue(Grid.RowProperty, 1);
            }





            //set frameskip option
            frameSkipPicker.ItemsSource = frameskiplist;
            powerFrameSkipPicker.ItemsSource = frameskiplist2;
            turboFrameSkipPicker.ItemsSource = frameskiplist2;
            aspectRatioPicker.ItemsSource = aspectRatioList;
            orientationPicker.ItemsSource = orientationList;

            ReadSettings();

        }

        protected override void OnNavigatedTo(NavigationEventArgs e)
        {
            //use MOGA controller needs to be set here incase we are coming back from purchase page
            this.toggleUseMogaController.IsChecked = EmulatorSettings.Current.UseMogaController;

            if (EmulatorSettings.Current.UseMogaController)
                MappingBtn.Visibility = Visibility.Visible;
            else
                MappingBtn.Visibility = Visibility.Collapsed;

            //in case return from image chooser page
            if (shouldUpdateBackgroud)
            {

                //manually update background and signal main page
                this.UpdateBackgroundImage();
                MainPage.shouldUpdateBackgroud = true;

                shouldUpdateBackgroud = false;

            }
            base.OnNavigatedTo(e);
        }


        protected override void OnBackKeyPress(System.ComponentModel.CancelEventArgs e)
        {
            //Check if the PopUp window is open
            if (popupWindow != null && popupWindow.IsOpen)
            {
                //Close the PopUp Window
                popupWindow.IsOpen = false;

                //Keep the back button from navigating away from the current page
                e.Cancel = true;
            }

            else
            {
                //There is no PopUp open, use the back button normally
                base.OnBackKeyPress(e);
            }

        }

        private void ReadSettings()
        {
            EmulatorSettings emuSettings = EmulatorSettings.Current;

            //this.vcontrollerPosSwitch.IsChecked = emuSettings.VirtualControllerOnTop;
            this.enableSoundSwitch.IsChecked = emuSettings.SoundEnabled;
            this.lowFreqSwitch.IsChecked = emuSettings.LowFrequencyMode;
            //this.stretchToggle.IsChecked = emuSettings.FullscreenStretch;
            this.scaleSlider.Value = emuSettings.ControllerScale;
            this.buttonScaleSlider.Value = emuSettings.ButtonScale;
            this.opacitySlider.Value = emuSettings.ControllerOpacity;
            this.imageScaleSlider.Value = emuSettings.ImageScaling;
            this.deadzoneSlider.Value = emuSettings.Deadzone;
            this.syncSoundSwitch.IsChecked = emuSettings.SynchronizeAudio;
            this.confirmationSwitch.IsChecked = emuSettings.HideConfirmationDialogs;
            this.autoIncSwitch.IsChecked = emuSettings.AutoIncrementSavestates;
            this.confirmationLoadSwitch.IsChecked = emuSettings.HideLoadConfirmationDialogs;
            //this.restoreLastStateSwitch.IsChecked = emuSettings.SelectLastState;
            this.cheatRestoreSwitch.IsChecked = emuSettings.RestoreOldCheatValues;
            this.manualSnapshotSwitch.IsChecked = emuSettings.ManualSnapshots;
            this.useColorButtonSwitch.IsChecked = emuSettings.UseColorButtons;

            this.showThreeDotsSwitch.IsChecked = App.metroSettings.ShowThreeDots;
            this.showLastPlayedGameSwitch.IsChecked = App.metroSettings.ShowLastPlayedGame;
            this.loadLastStateSwitch.IsChecked = emuSettings.AutoSaveLoad;

            if (App.metroSettings.BackgroundUri != null)
            {
                this.useBackgroundImageSwitch.IsChecked = true;
                this.backgroundOpacityPanel.Visibility = Visibility.Visible;
                this.ChooseBackgroundImageGrid.Visibility = Visibility.Visible;
            }
            else
            {
                this.useBackgroundImageSwitch.IsChecked = false;
                this.backgroundOpacityPanel.Visibility = Visibility.Collapsed;
                this.ChooseBackgroundImageGrid.Visibility = Visibility.Collapsed;
            }

            

            if (this.useColorButtonSwitch.IsChecked.Value)
                CustomizeBgcolorBtn.Visibility = System.Windows.Visibility.Visible;
            else
                CustomizeBgcolorBtn.Visibility = System.Windows.Visibility.Collapsed;

            
            

            this.Loaded += (o, e) =>
            {
               



                this.turboFrameSkipPicker.SelectedIndex = emuSettings.TurboFrameSkip;
                this.powerFrameSkipPicker.SelectedIndex = emuSettings.PowerFrameSkip;
                this.frameSkipPicker.SelectedIndex = Math.Min(emuSettings.FrameSkip + 1, this.frameSkipPicker.Items.Count - 1);
                this.aspectRatioPicker.SelectedIndex = (int)emuSettings.AspectRatio;
                this.orientationPicker.SelectedIndex = emuSettings.Orientation;

                this.dpadStyleBox.SelectedIndex = emuSettings.DPadStyle; //dpad
                this.assignPicker.SelectedIndex = emuSettings.CameraButtonAssignment; //camera assignment
                this.themePicker.SelectedIndex = App.metroSettings.ThemeSelection;

                this.backgroundOpacitySlider.Value = App.metroSettings.BackgroundOpacity * 100;

                initdone = true;
            };

        }


        private void enableSoundSwitch_Checked_1(object sender, RoutedEventArgs e)
        {
            if (this.initdone)
            {
                EmulatorSettings.Current.SoundEnabled = true;
            }
        }

        private void enableSoundSwitch_Unchecked_1(object sender, RoutedEventArgs e)
        {
            if (this.initdone)
            {
                EmulatorSettings.Current.SoundEnabled = false;
            }
        }

        private void lowFreqSwitch_Checked_1(object sender, RoutedEventArgs e)
        {
            if (this.initdone)
            {
                EmulatorSettings.Current.LowFrequencyMode = true;
            }
        }

        private void lowFreqSwitch_Unchecked_1(object sender, RoutedEventArgs e)
        {
            if (this.initdone)
            {
                EmulatorSettings.Current.LowFrequencyMode = false;
            }
        }

        private void opacitySlider_ValueChanged_1(object sender, RoutedPropertyChangedEventArgs<double> e)
        {
            if (this.initdone)
            {
                EmulatorSettings.Current.ControllerOpacity = (int) this.opacitySlider.Value;
            }
        }

        private void scaleSlider_ValueChanged_1(object sender, RoutedPropertyChangedEventArgs<double> e)
        {
            if (this.initdone)
            {
                EmulatorSettings.Current.ControllerScale = (int)this.scaleSlider.Value;
            }
        }

        private void ButtonScaleSlider_ValueChanged_1(object sender, RoutedPropertyChangedEventArgs<double> e)
        {
            if (this.initdone)
            {
                EmulatorSettings.Current.ButtonScale = (int)this.buttonScaleSlider.Value;
            }
        }

        private void imageScaleSlider_ValueChanged_1(object sender, RoutedPropertyChangedEventArgs<double> e)
        {
            if (this.initdone)
            {
                EmulatorSettings.Current.ImageScaling = (int)this.imageScaleSlider.Value;
            }
        }

        private void syncSoundSwitch_Checked_1(object sender, RoutedEventArgs e)
        {
            if (this.initdone)
            {
                EmulatorSettings.Current.SynchronizeAudio = true;
            }
        }

        private void syncSoundSwitch_Unchecked_1(object sender, RoutedEventArgs e)
        {
            if (this.initdone)
            {
                EmulatorSettings.Current.SynchronizeAudio = false;
            }
        }

        private void deadzoneSlider_ValueChanged_1(object sender, RoutedPropertyChangedEventArgs<double> e)
        {
            if (this.initdone)
            {
                EmulatorSettings.Current.Deadzone = (float)this.deadzoneSlider.Value;
            }
        }

        private void confirmationSwitch_Checked_1(object sender, RoutedEventArgs e)
        {
            if (this.initdone)
            {
                EmulatorSettings.Current.HideConfirmationDialogs = true;
            }
        }

        private void confirmationSwitch_Unchecked_1(object sender, RoutedEventArgs e)
        {
            if (this.initdone)
            {
                EmulatorSettings.Current.HideConfirmationDialogs = false;
            }
        }

        private void autoIncSwitch_Checked_1(object sender, RoutedEventArgs e)
        {
            if (this.initdone)
            {
                EmulatorSettings.Current.AutoIncrementSavestates = true;
            }
        }

        private void autoIncSwitch_Unchecked_1(object sender, RoutedEventArgs e)
        {
            if (this.initdone)
            {
                EmulatorSettings.Current.AutoIncrementSavestates = false;
            }
        }

        private void confirmationLoadSwitch_Checked_1(object sender, RoutedEventArgs e)
        {
            if (this.initdone)
            {
                EmulatorSettings.Current.HideLoadConfirmationDialogs = true;
            }
        }

        private void confirmationLoadSwitch_Unchecked_1(object sender, RoutedEventArgs e)
        {
            if (this.initdone)
            {
                EmulatorSettings.Current.HideLoadConfirmationDialogs = false;
            }
        }

        //private void restoreLastStateSwitch_Checked_1(object sender, RoutedEventArgs e)
        //{
        //    if (this.initdone)
        //    {
        //        EmulatorSettings.Current.SelectLastState = true;
        //    }
        //}

        //private void restoreLastStateSwitch_Unchecked_1(object sender, RoutedEventArgs e)
        //{
        //    if (this.initdone)
        //    {
        //        EmulatorSettings.Current.SelectLastState = false;
        //    }
        //}

        private void cheatRestoreSwitch_Checked(object sender, RoutedEventArgs e)
        {
            if (this.initdone)
            {
                EmulatorSettings.Current.RestoreOldCheatValues = true;
            }
        }

        private void cheatRestoreSwitch_Unchecked(object sender, RoutedEventArgs e)
        {
            if (this.initdone)
            {
                EmulatorSettings.Current.RestoreOldCheatValues = false;
            }
        }

        private void manualSnapshotSwitch_Checked(object sender, RoutedEventArgs e)
        {
            if (this.initdone)
            {
                EmulatorSettings.Current.ManualSnapshots = true;
            }
        }

        private void manualSnapshotSwitch_Unchecked(object sender, RoutedEventArgs e)
        {
            if (this.initdone)
            {
                EmulatorSettings.Current.ManualSnapshots = false;
            }
        }

        private void aspectRatioPicker_SelectionChanged_1(object sender, SelectionChangedEventArgs e)
        {
            if (this.initdone)
            {
                EmulatorSettings.Current.AspectRatio = (AspectRatioMode)aspectRatioPicker.SelectedIndex;
            }
        }


   

        private void orientationPicker_SelectionChanged_1(object sender, SelectionChangedEventArgs e)
        {
            if (this.initdone)
            {
                EmulatorSettings.Current.Orientation = orientationPicker.SelectedIndex;
            }
        }

        private void orientationBothRadio_Checked(object sender, RoutedEventArgs e)
        {
            if (this.initdone)
            {
                EmulatorSettings.Current.Orientation = 0;
            }
        }

        private void orientationLandscapeRadio_Checked(object sender, RoutedEventArgs e)
        {
            if (this.initdone)
            {
                EmulatorSettings.Current.Orientation = 1;
            }
        }

        private void orientationPortraitRadio_Checked(object sender, RoutedEventArgs e)
        {
            if (this.initdone)
            {
                EmulatorSettings.Current.Orientation = 2;
            }
        }

        private void assignPicker_SelectionChanged_1(object sender, SelectionChangedEventArgs e)
        {
            if (this.initdone)
            {
                EmulatorSettings.Current.CameraButtonAssignment = this.assignPicker.SelectedIndex;
            }
        }

        private void dpadStyleBox_SelectionChanged_1(object sender, SelectionChangedEventArgs e)
        {
            if (this.initdone)
            {
                EmulatorSettings.Current.DPadStyle = this.dpadStyleBox.SelectedIndex;
            }
        }

        private void frameSkipPicker_SelectionChanged_1(object sender, SelectionChangedEventArgs e)
        {
            if (this.initdone)
            {
                EmulatorSettings.Current.FrameSkip = (int)this.frameSkipPicker.SelectedIndex - 1;

            }
        }

        private void powerFrameSkipPicker_SelectionChanged_1(object sender, SelectionChangedEventArgs e)
        {
            if (this.initdone)
            {
                EmulatorSettings.Current.PowerFrameSkip = this.powerFrameSkipPicker.SelectedIndex;
            }
        }

        private void toggleUseMogaController_Tap(object sender, System.Windows.Input.GestureEventArgs e)
        {
            if (this.initdone)
            {
#if BETA
                EmulatorSettings.Current.UseMogaController = toggleUseMogaController.IsChecked.Value;
#else
                if (App.IsPremium)
                {
                    EmulatorSettings.Current.UseMogaController = toggleUseMogaController.IsChecked.Value;
                }
                else
                {
                    toggleUseMogaController.IsChecked = EmulatorSettings.Current.UseMogaController;

                    //prompt to buy
                    MessageBoxResult result = MessageBox.Show(AppResources.PremiumFeaturePromptText, AppResources.UnlockFeatureText, MessageBoxButton.OKCancel);

                    if (result == MessageBoxResult.OK)
                    {
                        NavigationService.Navigate(new Uri("/PurchasePage.xaml", UriKind.Relative));
                    }
                }
#endif
                if (EmulatorSettings.Current.UseMogaController)
                    MappingBtn.Visibility = Visibility.Visible;
                else
                    MappingBtn.Visibility = Visibility.Collapsed;

            }
        }

        private void TextBlock_Tap_1(object sender, System.Windows.Input.GestureEventArgs e)
        {
            WebBrowserTask wbtask = new WebBrowserTask();
            wbtask.Uri = new Uri("http://www.youtube.com/watch?v=YfqzZhcr__o");
            wbtask.Show();
        }

        private void deadzoneLock_Tap(object sender, System.Windows.Input.GestureEventArgs e)
        {
            if (deadzoneSlider.IsEnabled)
            {
                deadzoneSlider.IsEnabled = false;
                deadzoneImage.ImageSource = new BitmapImage(new Uri("Assets/Icons/appbar.lock.png", UriKind.Relative));
            }
            else
            {
                deadzoneSlider.IsEnabled = true;
                deadzoneImage.ImageSource = new BitmapImage(new Uri("Assets/Icons/appbar.unlock.png", UriKind.Relative));
            }
        }

        private void scaleLock_Tap(object sender, System.Windows.Input.GestureEventArgs e)
        {
            if (scaleSlider.IsEnabled)
            {
                scaleSlider.IsEnabled = false;
                scaleImage.ImageSource = new BitmapImage(new Uri("Assets/Icons/appbar.lock.png", UriKind.Relative));
            }
            else
            {
                scaleSlider.IsEnabled = true;
                scaleImage.ImageSource = new BitmapImage(new Uri("Assets/Icons/appbar.unlock.png", UriKind.Relative));
            }
        }

        private void opacityLock_Tap(object sender, System.Windows.Input.GestureEventArgs e)
        {
            if (opacitySlider.IsEnabled)
            {
                opacitySlider.IsEnabled = false;
                opacityImage.ImageSource = new BitmapImage(new Uri("Assets/Icons/appbar.lock.png", UriKind.Relative));
            }
            else
            {
                opacitySlider.IsEnabled = true;
                opacityImage.ImageSource = new BitmapImage(new Uri("Assets/Icons/appbar.unlock.png", UriKind.Relative));
            }
        }

        private void backgroundOpacityLock_Tap(object sender, System.Windows.Input.GestureEventArgs e)
        {
            if (backgroundOpacitySlider.IsEnabled)
            {
                backgroundOpacitySlider.IsEnabled = false;
                backgroundOpacityImage.ImageSource = new BitmapImage(new Uri("Assets/Icons/appbar.lock.png", UriKind.Relative));
            }
            else
            {
                backgroundOpacitySlider.IsEnabled = true;
                backgroundOpacityImage.ImageSource = new BitmapImage(new Uri("Assets/Icons/appbar.unlock.png", UriKind.Relative));
            }
        }

        private void buttonScaleLock_Tap(object sender, System.Windows.Input.GestureEventArgs e)
        {
            if (buttonScaleSlider.IsEnabled)
            {
                buttonScaleSlider.IsEnabled = false;
                buttonScaleImage.ImageSource = new BitmapImage(new Uri("Assets/Icons/appbar.lock.png", UriKind.Relative));
            }
            else
            {
                buttonScaleSlider.IsEnabled = true;
                buttonScaleImage.ImageSource = new BitmapImage(new Uri("Assets/Icons/appbar.unlock.png", UriKind.Relative));
            }
        }

        private void CPositionPortraitBtn_Click(object sender, RoutedEventArgs e)
        {
            NavigationService.Navigate(new Uri("/CustomizeControllerPage.xaml?orientation=2", UriKind.Relative));
        }

        private void CPositionLandscapeBtn_Click(object sender, RoutedEventArgs e)
        {
            NavigationService.Navigate(new Uri("/CustomizeControllerPage.xaml?orientation=0", UriKind.Relative));
        }

        private void turboFrameSkipPicker_SelectionChanged_1(object sender, SelectionChangedEventArgs e)
        {
            if (this.initdone)
            {
                EmulatorSettings.Current.TurboFrameSkip = this.turboFrameSkipPicker.SelectedIndex;
            }
        }

        private void useColorButtonSwitch_Click(object sender, RoutedEventArgs e)
        {
            if (this.useColorButtonSwitch.IsChecked.Value)
                CustomizeBgcolorBtn.Visibility = System.Windows.Visibility.Visible;
            else
                CustomizeBgcolorBtn.Visibility = System.Windows.Visibility.Collapsed;

            if (this.initdone)
            {
                EmulatorSettings.Current.UseColorButtons = this.useColorButtonSwitch.IsChecked.Value;
            }
        }

        private void CustomizeBgcolorBtn_Click(object sender, RoutedEventArgs e)
        {
            if (App.IsPremium || App.HasAds == false)
            {


                //disable current page
                this.IsHitTestVisible = false;
                //this.Content.Visibility = Visibility.Collapsed;

                //create new popup instance

                popupWindow = new Popup();

                popupWindow.Child = new ColorChooserControl();

                popupWindow.VerticalOffset = 130;
                popupWindow.HorizontalOffset = 10;
                popupWindow.IsOpen = true;

                popupWindow.Closed += (s1, e1) =>
                {
                    this.IsHitTestVisible = true;
                    //this.Content.Visibility = Visibility.Visible;

                };
            }
            else
            {
                //prompt to buy
                MessageBoxResult result = MessageBox.Show(AppResources.PremiumFeaturePromptText, AppResources.UnlockFeatureText, MessageBoxButton.OKCancel);

                if (result == MessageBoxResult.OK)
                {
                    NavigationService.Navigate(new Uri("/PurchasePage.xaml", UriKind.Relative));
                }
            }

        }

        private void MappingBtn_Click(object sender, RoutedEventArgs e)
        {
            NavigationService.Navigate(new Uri("/MogaMappingPage.xaml", UriKind.Relative));
        }

        private void themePicker_SelectionChanged_1(object sender, SelectionChangedEventArgs e)
        {
            if (this.initdone)
            {
                App.metroSettings.ThemeSelection = themePicker.SelectedIndex;

                App.MergeCustomColors();
                //CustomMessageBox msgbox = new CustomMessageBox();
                //msgbox.Background = (SolidColorBrush)App.Current.Resources["PhoneChromeBrush"];
                //msgbox.Foreground = (SolidColorBrush)App.Current.Resources["PhoneForegroundBrush"];
                //msgbox.Message = AppResources.RestartPromptText;
                //msgbox.Caption = AppResources.RestartPromptTitle;
                //msgbox.LeftButtonContent = "OK";
                //msgbox.Show();
            }
        }

        private void showThreeDotsSwitch_Click(object sender, RoutedEventArgs e)
        {
            if  (this.initdone)
            {
                App.metroSettings.ShowThreeDots = showThreeDotsSwitch.IsChecked.Value;
            }
        }

        private void useBackgroundImageSwitch_Click(object sender, RoutedEventArgs e)
        {
            if (this.initdone)
            {
                if (useBackgroundImageSwitch.IsChecked.Value)
                {
                    if (App.metroSettings.UseDefaultBackground)
                        App.metroSettings.BackgroundUri = FileHandler.DEFAULT_BACKGROUND_IMAGE;
                    else
                        App.metroSettings.BackgroundUri = "CustomBackground.jpg";

                    this.backgroundOpacityPanel.Visibility = Visibility.Visible;
                    this.ChooseBackgroundImageGrid.Visibility = Visibility.Visible;

                    
                }
                else
                {
                    App.metroSettings.BackgroundUri = null;
                    this.backgroundOpacityPanel.Visibility = Visibility.Collapsed;
                    this.ChooseBackgroundImageGrid.Visibility = Visibility.Collapsed;

                    
                }

                //manually update background (can't find a better way)
                this.UpdateBackgroundImage();

                //signal main page
                MainPage.shouldUpdateBackgroud = true;
            }
        }

        private void backgroundOpacitySlider_ValueChanged_1(object sender, RoutedPropertyChangedEventArgs<double> e)
        {
            if (this.initdone)
            {
                App.metroSettings.BackgroundOpacity = this.backgroundOpacitySlider.Value / 100f;

                //manually update background (can't find a better way)
                this.UpdateBackgroundImage();

                //signal main page
                MainPage.shouldUpdateBackgroud = true;
            }
        }

        private void ChooseBackgroundImageBtn_Click(object sender, RoutedEventArgs e)
        {
            if (App.IsPremium)
            {
                PhotoChooserTask task = new PhotoChooserTask();
                task.Completed += photoChooserTask_Completed;
                task.ShowCamera = true;

                task.Show();
            }
            else
            {
                //prompt to buy
                MessageBoxResult result = MessageBox.Show(AppResources.PremiumFeaturePromptText, AppResources.UnlockFeatureText, MessageBoxButton.OKCancel);

                if (result == MessageBoxResult.OK)
                {
                    NavigationService.Navigate(new Uri("/PurchasePage.xaml", UriKind.Relative));
                }
            }


            
        }

        private void photoChooserTask_Completed(object sender, PhotoResult e)
        {
            if (e.TaskResult == TaskResult.OK)
            {
                BitmapImage bmp = new BitmapImage();
                bmp.SetSource(e.ChosenPhoto);  //this does not have info about width and length

                WriteableBitmap wb = new WriteableBitmap(bmp); //this has info about width and length

                ImageCroppingPage.wbSource = wb;

               
                NavigationService.Navigate(new Uri("/ImageCroppingPage.xaml", UriKind.Relative));

                
                
            
            }
        }



        private void ResetBackgroundImageBtn_Click(object sender, RoutedEventArgs e)
        {
            App.metroSettings.BackgroundUri = FileHandler.DEFAULT_BACKGROUND_IMAGE;
            App.metroSettings.UseDefaultBackground = true;

            //manually update background and signal main page
            this.UpdateBackgroundImage();
            MainPage.shouldUpdateBackgroud = true;

        }



        private void UpdateBackgroundImage()
        {
            if (App.metroSettings.BackgroundUri != null)
            {
                pivot.Background = new ImageBrush
                {
                    Opacity = App.metroSettings.BackgroundOpacity,
                    Stretch = Stretch.None,
                    AlignmentX = System.Windows.Media.AlignmentX.Center,
                    AlignmentY = System.Windows.Media.AlignmentY.Top,
                    ImageSource = FileHandler.getBitmapImage(App.metroSettings.BackgroundUri, FileHandler.DEFAULT_BACKGROUND_IMAGE)


                };
            }
            else
            {
                pivot.Background = null;
            }
        }

        private void showLastPlayedGameSwitch_Click(object sender, RoutedEventArgs e)
        {
            if (this.initdone)
            {
                App.metroSettings.ShowLastPlayedGame =  showLastPlayedGameSwitch.IsChecked.Value;
            }
        }

        private void loaLastStateSwitch_Click(object sender, RoutedEventArgs e)
        {
            if (this.initdone)
            {
                EmulatorSettings.Current.AutoSaveLoad = loadLastStateSwitch.IsChecked.Value;
            }
        }


        

       
    }
}