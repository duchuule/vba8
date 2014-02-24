using System;
using System.Diagnostics;
using System.ComponentModel;
using System.IO.IsolatedStorage;
using System.Globalization;
using System.Windows.Media;

namespace PhoneDirect3DXamlAppInterop
{
    public class AppSettings: INotifyPropertyChanged
    {

        // Our isolated storage settings
        IsolatedStorageSettings isolatedStore;

        //the following keys are use in metro mode only
        const String ThemeSelectionKey = "ThemeSelectionKey";
        const String ShowThreeDotsKey = "ShowThreeDotsKey";
        const String BackgroundUriKey = "BackgroundUriKey";
        const String BackgroundOpacityKey = "BackgroundOpacityKey";
        const String UseDefaultBackgroundKey = "UseDefaultBackgroundKey";
        const String ShowLastPlayedGameKey = "ShowLastPlayedGameKey";
        const String LastIPAddressKey = "LastIPAddressKey";
        const String LastTimeoutKey = "LastTimeoutKey";
        const String LoadLastStateKey = "LoadLastStateKey";  //abandon
        const String LoadLastState2Key = "LoadLastState2Key";

        /// <summary>
        /// Constructor that gets the application settings.
        /// </summary>
        public AppSettings()
        {
            try
            {
                // Get the settings for this application.
                isolatedStore = IsolatedStorageSettings.ApplicationSettings;
            }
            catch (Exception e)
            {
                Debug.WriteLine("Exception while using IsolatedStorageSettings: " + e.ToString());
            }
        }

        /// <summary>
        /// Update a setting value for our application. If the setting does not
        /// exist, then add the setting.
        /// </summary>
        /// <param name="Key"></param>
        /// <param name="value"></param>
        /// <returns></returns>
        public bool AddOrUpdateValue(string Key, Object value)
        {
            bool valueChanged = false;

            // If the key exists
            if (isolatedStore.Contains(Key))
            {
                // If the value has changed
                if (isolatedStore[Key] != value)
                {
                    // Store the new value
                    isolatedStore[Key] = value;
                    valueChanged = true;
                }
            }
            // Otherwise create the key.
            else
            {
                isolatedStore.Add(Key, value);
                valueChanged = true;
            }

            return valueChanged;
        }


        /// <summary>
        /// Get the current value of the setting, or if it is not found, set the 
        /// setting to the default setting.
        /// </summary>
        /// <typeparam name="valueType"></typeparam>
        /// <param name="Key"></param>
        /// <param name="defaultValue"></param>
        /// <returns></returns>
        public valueType GetValueOrDefault<valueType>(string Key, valueType defaultValue)
        {
            valueType value;

            // If the key exists, retrieve the value.
            if (isolatedStore.Contains(Key))
            {
                value = (valueType)isolatedStore[Key];
            }
            // Otherwise, use the default value.
            else
            {
                value = defaultValue;
            }

            return value;
        }


        /// <summary>
        /// Save the settings.
        /// </summary>
        public void Save()
        {
            isolatedStore.Save();
        }


        public int ThemeSelection
        {
            get
            {
                return GetValueOrDefault<int>(ThemeSelectionKey, 0);
            }
            set
            {
                AddOrUpdateValue(ThemeSelectionKey, value);
                Save();
            }
        }


        public bool ShowThreeDots
        {
            get
            {
                return GetValueOrDefault<bool>(ShowThreeDotsKey, true);
            }
            set
            {
                AddOrUpdateValue(ShowThreeDotsKey, value);
                Save();
            }
        }

        public String BackgroundUri
        {
            get
            {
                return GetValueOrDefault<String>(BackgroundUriKey, FileHandler.DEFAULT_BACKGROUND_IMAGE);
            }
            set
            {
                AddOrUpdateValue(BackgroundUriKey, value);
                Save();
                NotifyPropertyChanged("BackgroundUri");
            }
        }


        public double BackgroundOpacity
        {
            get
            {
                return GetValueOrDefault<double>(BackgroundOpacityKey, 0.2);
            }
            set
            {
                AddOrUpdateValue(BackgroundOpacityKey, value);
                Save();
            }
        }

        public bool UseDefaultBackground
        {
            get
            {
                return GetValueOrDefault<bool>(UseDefaultBackgroundKey, true);
            }
            set
            {
                AddOrUpdateValue(UseDefaultBackgroundKey, value);
                Save();
            }
        }

        public bool ShowLastPlayedGame
        {
            get
            {
                return GetValueOrDefault<bool>(ShowLastPlayedGameKey, true);
            }
            set
            {
                AddOrUpdateValue(ShowLastPlayedGameKey, value);
                Save();
            }
        }

        public string LastIPAddress
        {
            get
            {
                return GetValueOrDefault<string>(LastIPAddressKey, "");
            }
            set
            {
                AddOrUpdateValue(LastIPAddressKey, value);
                Save();
            }
        }


        public int LastTimeout
        {
            get
            {
                return GetValueOrDefault<int>(LastTimeoutKey, 3000);
            }
            set
            {
                AddOrUpdateValue(LastTimeoutKey, value);
                Save();
            }
        }

        public bool LoadLastState
        {
            get
            {
                return GetValueOrDefault<bool>(LoadLastStateKey, false);
            }
            set
            {
                AddOrUpdateValue(LoadLastStateKey, value);
                Save();
            }
        }

        public bool LoadLastState2
        {
            get
            {
                return GetValueOrDefault<bool>(LoadLastState2Key, false);
            }
            set
            {
                AddOrUpdateValue(LoadLastState2Key, value);
                Save();
            }
        }


        #region INotifyPropertyChanged Members

        public event PropertyChangedEventHandler PropertyChanged;

        // Used to notify that a property changed
        private void NotifyPropertyChanged(string propertyName)
        {
            if (PropertyChanged != null)
            {
                PropertyChanged(this, new PropertyChangedEventArgs(propertyName));
            }
        }

        #endregion


    }
}
