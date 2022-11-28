# Sign app with hardened runtime and audio entitlement
/usr/bin/codesign --force -s "Developer ID Application: Timothy Schoen (7SV7JPRR2L)" --options runtime $1/*.app

# Create .dmg file so we can staple it
hdiutil create -volname RecipherSettingsTool-macOS-Universal -srcfolder $1 -ov -format UDZO RecipherSettingsTool-macOS-Universal.dmg

# Notarise
xcrun notarytool store-credentials "notary_login" --apple-id ${AC_USERNAME} --password ${AC_PASSWORD} --team-id "7SV7JPRR2L"
xcrun notarytool submit RecipherSettingsTool-macOS-Universal.dmg --keychain-profile "notary_login" --wait
xcrun stapler staple "RecipherSettingsTool-macOS-Universal.dmg"