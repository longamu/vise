git checkout develop -- src/
tar -czvf vise-1.0.0.tar.gz src/
mv vise-1.0.0.tar.gz public/release/vise-1.0.0.tar.gz
git rm -fr src/
