clear

echo "Deleting build/ folder"
rm -rf build/
echo "Creating build/ folder"
mkdir build
cd build
echo "Entering build/ folder"

if [ "$1" == "debug" ];
    then
        echo "Build for debug with G3LOG"
        cmake -DCMAKE_BUILD_TYPE=Debug -DENABLE_G3LOG=ON ..
        make -j 8
elif [ "$1" == "production" ]; 
    then
        echo "Build for production"
        cmake -DCMAKE_BUILD_TYPE=Release -DENABLE_G3LOG=OFF ..
        make VERBOSE=1 -j 8
else
        echo "Build for releaase with G3LOG"
        cmake -DCMAKE_BUILD_TYPE=Release -DENABLE_G3LOG=ON ..
        make VERBOSE=1 -j 8
fi
