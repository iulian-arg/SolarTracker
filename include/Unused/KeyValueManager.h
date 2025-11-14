#ifndef KEYVALUE_MANAGER_H
#define KEYVALUE_MANAGER_H

#include <Preferences.h>

class KeyValueManager
{
private:
    Preferences _preferences;
    const char *_namespaceName = "default";

public:
    KeyValueManager(const char *namespaceName);
    void SetupKVM();
    bool putString(const char *key, const String &value);
    String getString(const char *key, const String &defaultValue = "");
    bool putInt(const char *key, int value);
    int getInt(const char *key, int defaultValue = 0);
    bool putBool(const char *key, bool value);
    bool getBool(const char *key, bool defaultValue = false);
    bool remove(const char *key);
    bool clear();

    void SetupKVM()
    {
        _preferences.begin(_namespaceName, false); // Open preferences in read-write mode
    }
};

#endif // KEYVALUE_MANAGER_H