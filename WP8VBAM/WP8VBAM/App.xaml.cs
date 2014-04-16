using System;
using System.Collections.Generic;
using System.Linq;
using System.Net;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Animation;
using System.Windows.Navigation;
using System.Windows.Shapes;
using Microsoft.Phone.Controls;
using Microsoft.Phone.Shell;
using Windows.ApplicationModel;
using Windows.ApplicationModel.Core;
using PhoneDirect3DXamlAppComponent;
using Windows.ApplicationModel.Store;
using Store = Windows.ApplicationModel.Store;
using System.Collections;
using System.IO.IsolatedStorage;
using Windows.Networking.Sockets;
using Microsoft.Phone.Info;
using System.Security.Cryptography;
using System.Text;
using System.IO;
using Microsoft.Live;
using Microsoft.Live.Controls;
using System.Threading;
using System.Windows.Markup;
using System.Diagnostics;

using PhoneDirect3DXamlAppInterop.Resources;
using PhoneDirect3DXamlAppInterop.Database;

namespace PhoneDirect3DXamlAppInterop
{
    public partial class App : Application
    {
        public static bool IsTrial
        { get; private set; }

        public static int APP_VERSION = 2;

        public static bool HasAds { get; private set; }
        public static bool IsPremium { get; private set; }
        public static LiveConnectSession session;

        public static AppSettings metroSettings = new AppSettings();

        public static StreamSocket linkSocket;           // The socket object used to communicate with a peer

        public static DateTime LastAutoBackupTime;

        public static string exportFolderID;



        public static void DetermineIsTrail()
        {

            App.LastAutoBackupTime = DateTime.Now;

            HasAds = true;
            IsPremium = false;


#if BETA
            IsPremium = true;
#endif
            try
            {
                EmulatorSettings.Current.IsTrial = IsTrial;
            }
            catch (Exception) { }

            

            if (CurrentApp.LicenseInformation.ProductLicenses["noads_premium"].IsActive)
            {
                HasAds = false;
                IsPremium = true;
                return; //no need to check for other 2 licenses
            }

            //get information on in-app purchase
            if (CurrentApp.LicenseInformation.ProductLicenses["removeads"].IsActive)
                HasAds = false;


            if (CurrentApp.LicenseInformation.ProductLicenses["premiumfeatures"].IsActive)
                IsPremium = true;

            //check if a promotion code exists
            if (metroSettings.PromotionCode != null && metroSettings.PromotionCode != "")
            {
                byte[] byteCode;
                try
                {
                    byteCode = Convert.FromBase64String(metroSettings.PromotionCode);
                }
                catch (Exception)
                {
                    return;
                }


                string dataString = Convert.ToBase64String(DeviceExtendedProperties.GetValue("DeviceUniqueId") as byte[]) + "_noads_premium";

                // Create byte arrays to hold original, encrypted, and decrypted data.

                UTF8Encoding ByteConverter = new UTF8Encoding();
                byte[] originalData = ByteConverter.GetBytes(dataString);



                RSACryptoServiceProvider RSAalg = new RSACryptoServiceProvider(2048);

                Stream src = Application.GetResourceStream(new Uri("Assets/VBA8publicKey.xml", UriKind.Relative)).Stream;
                using (StreamReader sr = new StreamReader(src))
                {
                    string text = sr.ReadToEnd();
                    RSAalg.FromXmlString(text);
                }

                RSAParameters Key = RSAalg.ExportParameters(false);

                if (PurchasePage.VerifySignedHash(originalData, byteCode, Key))
                {
                    HasAds = false;
                    IsPremium = true;
                }

            }
            
        }


        public static void MergeCustomColors()
        {
            var dictionaries = new ResourceDictionary();
            string source;
            Color systemTrayColor;
            SolidColorBrush brush;




            //remove then add, stupid silverlight does not allow to change value
            App.Current.Resources.Remove("CustomForegroundColor");
            App.Current.Resources.Remove("CustomChromeColor");

            if (metroSettings.ThemeSelection == 0)
            {
                source = String.Format("/CustomTheme/LightTheme.xaml");

                App.Current.Resources.Add("CustomChromeColor", Color.FromArgb(255, 221, 221, 221)); //same as PhoneChromeColor
                App.Current.Resources.Add("CustomForegroundColor", Color.FromArgb(0xDE, 0, 0, 0)); //same as PhoneForegroundColor

            }
            else
            {
                source = String.Format("/CustomTheme/DarkTheme.xaml");

                App.Current.Resources.Add("CustomChromeColor", Color.FromArgb(255, 0x1f, 0x1f, 0x1f));
                App.Current.Resources.Add("CustomForegroundColor", Color.FromArgb(255, 255, 255, 255));


            }

            //system color
            systemTrayColor = Color.FromArgb(255, 0x4d, 0x3a, 0x89);
#if GBC
            systemTrayColor = Color.FromArgb(255, 0xb6, 0x1e, 0x45);
#endif
            App.Current.Resources.Remove("SystemTrayColor");
            App.Current.Resources.Add("SystemTrayColor", systemTrayColor);

            //brushes

            SolidColorBrush brush1 = App.Current.Resources["HeaderBackgroundBrush"] as SolidColorBrush;
            brush1.Color = systemTrayColor;
            brush1.Opacity = 0.7;


            SolidColorBrush brush3 = App.Current.Resources["HeaderForegroundBrush"] as SolidColorBrush;
            brush3.Color = Colors.White;
            brush3.Opacity = 1.0;


            SolidColorBrush brush2 = App.Current.Resources["ListboxBackgroundBrush"] as SolidColorBrush;
            brush2.Color = systemTrayColor;
#if GBC
            brush2.Opacity = 0.05;
#else
            brush2.Opacity = 0.1;
#endif


            var themeStyles = new ResourceDictionary { Source = new Uri(source, UriKind.Relative) };
            dictionaries.MergedDictionaries.Add(themeStyles);


            ResourceDictionary appResources = App.Current.Resources;
            foreach (DictionaryEntry entry in dictionaries.MergedDictionaries[0])
            {
                SolidColorBrush colorBrush = entry.Value as SolidColorBrush;
                SolidColorBrush existingBrush = appResources[entry.Key] as SolidColorBrush;
                if (existingBrush != null && colorBrush != null)
                {
                    existingBrush.Color = colorBrush.Color;
                }
            }

        }
        


        /// <summary>
        /// Provides easy access to the root frame of the Phone Application.
        /// </summary>
        /// <returns>The root frame of the Phone Application.</returns>
        public PhoneApplicationFrame RootFrame { get; private set; }



        // Set to true when the page navigation is being reset 
        bool wasRelaunched = false;

        // set to true when current rom is different from previous rom
        bool mustClearPagestack = false;

        IsolatedStorageSettings settings = IsolatedStorageSettings.ApplicationSettings;

        /// <summary>
        /// Constructor for the Application object.
        /// </summary>
        public App()
        {
            // Global handler for uncaught exceptions.
            UnhandledException += Application_UnhandledException;

            // Standard Silverlight initialization
            InitializeComponent();

            //merge custom theme
            MergeCustomColors();

            // Phone-specific initialization
            InitializePhoneApplication();

            // Language display initialization
            InitializeLanguage();

            // Show graphics profiling information while debugging.
            if (System.Diagnostics.Debugger.IsAttached)
            {
                // Display the current frame rate counters.
                Application.Current.Host.Settings.EnableFrameRateCounter = true;

                // Show the areas of the app that are being redrawn in each frame.
                //Application.Current.Host.Settings.EnableRedrawRegions = true;

                // Enable non-production analysis visualization mode,
                // which shows areas of a page that are handed off to GPU with a colored overlay.
                //Application.Current.Host.Settings.EnableCacheVisualization = true;

                // Disable the application idle detection by setting the UserIdleDetectionMode property of the
                // application's PhoneApplicationService object to Disabled.
                // Caution:- Use this under debug mode only. Application that disables user idle detection will continue to run
                // and consume battery power when the user is not using the phone.
                PhoneApplicationService.Current.UserIdleDetectionMode = IdleDetectionMode.Disabled;
            }

            //create data base
            ROMDatabase.Current.Initialize();

            //load collection
            ROMDatabase.Current.LoadCollectionsFromDatabase();
        }



        // Code to execute when the application is launching (eg, from Start)
        // This code will not execute when the application is reactivated
        private void Application_Launching(object sender, LaunchingEventArgs e)
        {
            DetermineIsTrail();
        }

        // Code to execute when the application is activated (brought to foreground)
        // This code will not execute when the application is first launched
        private void Application_Activated(object sender, ActivatedEventArgs e)
        {
            DetermineIsTrail();
        }

        // Code to execute when the application is deactivated (sent to background)
        // This code will not execute when the application is closing
        private void Application_Deactivated(object sender, DeactivatedEventArgs e)
        {
            // When the applicaiton is deactivated, save the current deactivation settings to isolated storage
            SaveCurrentDeactivationSettings();

        }

        // Code to execute when the application is closing (eg, user hit Back)
        // This code will not execute when the application is deactivated
        private void Application_Closing(object sender, ClosingEventArgs e)
        {
            // When the application closes, delete any deactivation settings from isolated storage
            RemoveCurrentDeactivationSettings();
        }

        // Code to execute if a navigation fails
        private void RootFrame_NavigationFailed(object sender, NavigationFailedEventArgs e)
        {
            if (System.Diagnostics.Debugger.IsAttached)
            {
                // A navigation has failed; break into the debugger
                System.Diagnostics.Debugger.Break();
            }
        }

        // Code to execute on Unhandled Exceptions
        private void Application_UnhandledException(object sender, ApplicationUnhandledExceptionEventArgs e)
        {
            if (System.Diagnostics.Debugger.IsAttached)
            {
                // An unhandled exception has occurred; break into the debugger
                System.Diagnostics.Debugger.Break();
            }
        }

        #region Phone application initialization

        // Avoid double-initialization
        private bool phoneApplicationInitialized = false;

        // Do not add any additional code to this method
        private void InitializePhoneApplication()
        {
            if (phoneApplicationInitialized)
                return;

            // Create the frame but don't set it as RootVisual yet; this allows the splash
            // screen to remain active until the application is ready to render.
            RootFrame = new PhoneApplicationFrame();
            RootFrame.Navigated += CompleteInitializePhoneApplication;

            RootFrame.UriMapper = new GBAUriMapper();

            // Handle navigation failures
            RootFrame.NavigationFailed += RootFrame_NavigationFailed;

            // Handle reset requests for clearing the backstack
            RootFrame.Navigated += CheckForResetNavigation;

            // Monitor deep link launching 
            RootFrame.Navigating += RootFrame_Navigating;

            // Ensure we don't initialize again
            phoneApplicationInitialized = true;
        }


        // Event handler for the Navigating event of the root frame. Use this handler to modify
        // the default navigation behavior.
        void RootFrame_Navigating(object sender, NavigatingCancelEventArgs e)
        {



            if (e.NavigationMode == NavigationMode.Reset)
            {
                // This block will execute if the current navigation is a relaunch.
                // If so, another navigation will be coming, so this records that a relaunch just happened
                // so that the next navigation can use this info.
                wasRelaunched = true;
            }
            else if (e.NavigationMode == NavigationMode.New && wasRelaunched)
            {
                // This block will run if the previous navigation was a relaunch
                wasRelaunched = false;
                mustClearPagestack = true;

                if (e.Uri.ToString().Contains(FileHandler.ROM_URI_STRING + "="))
                {
                    // This block will run if the launch Uri contains "rom=" which
                    // was specified when the secondary tile was created in FileHandler.cs

                    
                    //check to see if the rom to be launched is the same as the rom currently being played
                    if (CheckLastRomPlayed(HttpUtility.UrlDecode(e.Uri.ToString()) ))
                        mustClearPagestack = false;
                    else
                        mustClearPagestack = true;
                }
                else if (e.Uri.ToString().Contains("/MainPage.xaml"))
                {
                    
                    // This block will run if the navigation Uri is the main page
                    mustClearPagestack = false;

                }

                if (mustClearPagestack == false)
                {
                    //The app was previously launched via Main Tile and relaunched via Main Tile. Cancel the navigation to resume.
                    e.Cancel = true;
                    RootFrame.Navigated -= ClearBackStackAfterReset;
                }
            }
        }

        // Do not add any additional code to this method
        private void CompleteInitializePhoneApplication(object sender, NavigationEventArgs e)
        {
            // Set the root visual to allow the application to render
            if (RootVisual != RootFrame)
                RootVisual = RootFrame;

            // Remove this handler since it is no longer needed
            RootFrame.Navigated -= CompleteInitializePhoneApplication;
        }

        private void CheckForResetNavigation(object sender, NavigationEventArgs e)
        {
            // If the app has received a 'reset' navigation, then we need to check
            // on the next navigation to see if the page stack should be reset
            if (e.NavigationMode == NavigationMode.Reset)
            {
                RootFrame.Navigated += ClearBackStackAfterReset;
            }
        }

        private void ClearBackStackAfterReset(object sender, NavigationEventArgs e)
        {
            // Unregister the event so it doesn't get called again
            RootFrame.Navigated -= ClearBackStackAfterReset;

            // Only clear the stack for 'new' (forward) navigations
            if (e.NavigationMode != NavigationMode.New)
                return;

            // For UI consistency, clear the entire page stack
            while (RootFrame.RemoveBackEntry() != null)
            {
                ; // do nothing
            }
        }

        #endregion


        // Initialize the app's font and flow direction as defined in its localized resource strings.
        //
        // To ensure that the font of your application is aligned with its supported languages and that the
        // FlowDirection for each of those languages follows its traditional direction, ResourceLanguage
        // and ResourceFlowDirection should be initialized in each resx file to match these values with that
        // file's culture. For example:
        //
        // AppResources.es-ES.resx
        //    ResourceLanguage's value should be "es-ES"
        //    ResourceFlowDirection's value should be "LeftToRight"
        //
        // AppResources.ar-SA.resx
        //     ResourceLanguage's value should be "ar-SA"
        //     ResourceFlowDirection's value should be "RightToLeft"
        //
        // For more info on localizing Windows Phone apps see http://go.microsoft.com/fwlink/?LinkId=262072.
        //
        private void InitializeLanguage()
        {
            try
            {
                // Set the font to match the display language defined by the
                // ResourceLanguage resource string for each supported language.
                //
                // Fall back to the font of the neutral language if the Display
                // language of the phone is not supported.
                //
                // If a compiler error is hit then ResourceLanguage is missing from
                // the resource file.
                RootFrame.Language = XmlLanguage.GetLanguage(AppResources.ResourceLanguage);

                // Set the FlowDirection of all elements under the root frame based
                // on the ResourceFlowDirection resource string for each
                // supported language.
                //
                // If a compiler error is hit then ResourceFlowDirection is missing from
                // the resource file.
                FlowDirection flow = (FlowDirection)Enum.Parse(typeof(FlowDirection), AppResources.ResourceFlowDirection);
                RootFrame.FlowDirection = flow;
            }
            catch
            {
                // If an exception is caught here it is most likely due to either
                // ResourceLangauge not being correctly set to a supported language
                // code or ResourceFlowDirection is set to a value other than LeftToRight
                // or RightToLeft.

                if (Debugger.IsAttached)
                {
                    Debugger.Break();
                }

                throw;
            }
        }


        // Called when the app is deactivating. Saves the time of the deactivation and the 
        // session type of the app instance to isolated storage.
        public void SaveCurrentDeactivationSettings()
        {
            //get current page before deactivated
            Uri currentUri = RootFrame.CurrentSource;

            if (currentUri.ToString().Contains("EmulatorPage.xaml"))
            {
                if (metroSettings.AddOrUpdateValue("LastRomPlayed", EmulatorPage.currentROMEntry.FileName))
                {
                    metroSettings.Save();
                }
            }
            else
            {
                if (metroSettings.AddOrUpdateValue("LastRomPlayed", "no_rom_is_being_played"))
                {
                    metroSettings.Save();
                }
            }



        }


        // Called when the app is launched or closed. Removes all deactivation settings from
        // isolated storage
        public void RemoveCurrentDeactivationSettings()
        {
            metroSettings.RemoveValue("LastRomPlayed");
            metroSettings.Save();
        }

        // Helper method to determine if the rom being launched from deep link is the same as the rom currently being played (if any)
        //true if the same, false if different
        bool CheckLastRomPlayed(string currentUri)
        {

            if (metroSettings.Contains("LastRomPlayed"))
            {
                string lastRomPlayed = settings["LastRomPlayed"] as string;

                return currentUri.Contains(lastRomPlayed);
            }
            else
                return false;
        }
    }
}