using System;
using System.Diagnostics;
using System.ComponentModel;
using System.IO.IsolatedStorage;
using System.Globalization;
using System.Windows.Media;

namespace PhoneDirect3DXamlAppInterop
{
    public class AppSettings
    {

        // Our isolated storage settings
        IsolatedStorageSettings isolatedStore;

        //the following keys are use in metro mode only
        const String ThemeSelectionKey = "ThemeSelectionKey";
        const String ShowThreeDotsKey = "ShowThreeDotsKey";

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


    }
}
