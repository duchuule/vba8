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


#if BETA
            IsPremium = true;
#endif
            try
            {
                EmulatorSettings.Current.IsTrial = IsTrial;
            }
            catch (Exception) { }

            HasAds = true;
            IsPremium = false;

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



        


        /// <summary>
        /// Provides easy access to the root frame of the Phone Application.
        /// </summary>
        /// <returns>The root frame of the Phone Application.</returns>
        public PhoneApplicationFrame RootFrame { get; private set; }

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
        }

        // Code to execute when the application is closing (eg, user hit Back)
        // This code will not execute when the application is deactivated
        private void Application_Closing(object sender, ClosingEventArgs e)
        {

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

            // Ensure we don't initialize again
            phoneApplicationInitialized = true;
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
    }
}