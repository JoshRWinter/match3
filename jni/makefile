apk=match3-debug.apk

all:../assets/assets ../assets/aassets
	NDK_DEBUG=1 ndk-build -C ..
	ant debug -f ../build.xml
	adb install -r $(CURDIR)/../bin/$(apk)
	
../assets/assets:../assets_local/build.txt ../assets_local/*.tga
	cd ../assets_local/ && pack --build

#../assets/uiassets.pack:../assets_local/uibuild.txt ../assets_local/*.tga
#	cd ../assets_local/ && pack --build uibuild.txt

../assets/aassets:../assets_local/apack.txt ../assets_local/*.ogg
	cd ../assets_local/ && apack --build
