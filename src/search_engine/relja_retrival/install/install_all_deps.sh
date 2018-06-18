./build_offline.sh
./build_frontend.sh

echo "Installing jp_draw"

cd ../src/external/jp_draw
python setup.py build
sudo python setup.py install
cd -

echo ""
echo "Now install (see docs/Manual_for_Black_Box_Usage.pdf):"
echo "   FASTANN"
echo "   pypar"
echo ""
echo "Then install dkmeans_relja by running"
echo ""
echo "cd ../src/external/dkmeans_relja"
echo "python setup.py build"
echo "sudo python setup.py install"
