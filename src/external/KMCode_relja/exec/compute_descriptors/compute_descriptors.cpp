#include "compute_descriptors.h"

#include "../../ImageContent/imageContent.h"
#include "../../descriptor/descriptor.h"
#include "../../ttime/ttime.h"

float scale_mult = 3.0;

namespace KM_compute_descriptors {
int lib_main(int argc, char **argv){

  if(argc<2){
    cout << "Interest point descriptors implemented by Krystian.Mikolajczyk@inrialpes.fr\n";
    cout << "at INRIA Rhone-Alpes.[ref. www.inrialpes.fr/movi/people/Mikolajczyk/Affine]" <<endl;
    cout << "Additional edits by James Philbin <james@robots.ox.ac.uk>" << endl;
    cout << "Options:"<< endl;
    cout << "     -jla  - steerable filters " << endl;
    cout << "     -sift - sift [D. Lowe]" << endl;
    cout << "     -gloh - extended sift " << endl;
    cout << "     -mom  - moments " << endl;
    cout << "     -koen - differential invariants " << endl;
    cout << "     -cf   - complex filters [F. Schaffalitzky]" << endl;
    cout << "     -sc   - shape context " << endl;
    cout << "     -spin - spin "  << endl;
    cout << "     -pca - gradient pca [Y. Ke]"  << endl;
    cout << "     -cc - cross correlation"  << endl;
    cout << "     -i image.pgm  - input image pgm, ppm, png or jpg" << endl;
    cout << "     -p1 image.pgm.points - input regions format 1" << endl;
    cout << "     -p2 image.pgm.points - input regions format 2" << endl;
    cout << "     -o1 out.desc - saves descriptors in out.desc output format1" << endl;
    cout << "     -o2 out.desc - saves descriptors in out.desc output format2" << endl;
    cout << "     -o3 out.desc - JP: Saves as <x> <y> <theta> <a> <b> <c> <desc>..." << endl;
    cout << "     -noangle - computes rotation variant descriptors (no rotation esimation)" << endl;
    cout << "     -DR - draws regions in out.desc.pgm" << endl;
    cout << "     -c 255 - draws points in grayvalue [0,...,255]" << endl;
    cout << "     -scale-mult - JP: Scale to multiply the incoming points by [3.0]" << endl;
    cout <<"example:\n     "<< argv[0]<< " -jla -i image.pgm -p1 image.pgm.points -DR " << endl << endl;
    cout <<" file format 1:"<< endl;
    cout <<"vector_dimension" << endl;
    cout << "nb_of_descriptors" << endl;
    cout << "x y a b c desc_1 desc_2 ......desc_vector_dimension"<< endl << endl;
    cout <<"where a(x-u)(x-u)+2b(x-u)(y-v)+c(y-v)(y-v)=1 " <<  endl <<  endl;
    cout <<" file format 2:"<< endl;
    cout <<"vector_dimension"  << endl;
    cout <<"nb_of_descriptors" << endl;
    cout <<"x y cornerness scale angle point_type laplacian extremum_type mi11 mi12 mi21 mi22 desc_1 ...... desc_vector_dimension"<< endl << endl;
    cout <<"distance=(descA_1-descB_1)^2+...+(descA_vector_dimension-descB_vector_dimension)^2"<< endl;

    return -1;
  }

  char input[512];
  char input_desc[512];
  char output[512];
  int in=0,p=2,out=0,dr=0,color=255;
  for(int i=1;i<argc;i++){
    if(!strcmp(argv[i],"-i")){
      in = 1;sprintf(input,argv[i+1]);
    }else if(!strcmp(argv[i],"-p1")){
      p=1;
      sprintf(input_desc,argv[i+1]);
    }else if(!strcmp(argv[i],"-p2")){
      p=0;
      sprintf(input_desc,argv[i+1]);
    }else if (!strcmp(argv[i],"-scale-mult")) {
      scale_mult = atof(argv[i+1]);
    }
  }

  if(in==0 || p==2){
    cout << "no input file or descriptor not specified."<< endl;
    return -1;
  }

  #ifndef QUIET
  cout << "computing descriptors in image " << input << " for points in "<< input_desc<<"..."<< endl;
  #endif

  for(int i=1;i<argc;i++){
    if(!strcmp(argv[i],"-o1")){
      out=1;sprintf(output,argv[i+1]);
    }
    else if(!strcmp(argv[i],"-o2")){
      out=2;sprintf(output,argv[i+1]);
    }
    else if(!strcmp(argv[i],"-o3")){
      out=3; sprintf(output,argv[i+1]);
    }
  }
  DARY *image = new ImageContent(input);
  image->toGRAY();
  image->char2float();
  vector<CornerDescriptor*> desc;

  loadCorners(input_desc,desc,p);
  //computeAngle(image, desc);

  for(int i=1;i<argc;i++){
    if(!strcmp(argv[i],"-noangle")){
      for(uint c=0;c<desc.size();c++){
	desc[c]->setAngle(0.0);
      }
    }
  }


  long t;
  init_time(&t);

  for(int i=1;i<argc;i++){
    if(!strcmp(argv[i],"-jla")){
      if(!out)sprintf(output,"%s.jla",input_desc);
      computeJLADescriptors(image, desc);
    }else if(!strcmp(argv[i],"-sift")){
      if(!out)sprintf(output,"%s.sift",input_desc);
      computeSiftDescriptors(image, desc);
    }else if(!strcmp(argv[i],"-gloh")){
      if(!out)sprintf(output,"%s.gloh",input_desc);
      computeESiftDescriptors(image, desc);
    }else if(!strcmp(argv[i],"-cc")){
      if(!out)sprintf(output,"%s.cc",input_desc);
      computeCCDescriptors(image, desc);
    }else if(!strcmp(argv[i],"-mom")){
      if(!out)sprintf(output,"%s.mom",input_desc);
      computeMomentDescriptors(image, desc);
    }else if(!strcmp(argv[i],"-koen")){
      if(!out)sprintf(output,"%s.koen",input_desc);
      computeKoenDescriptors(image, desc);
    }else if(!strcmp(argv[i],"-cf")){
      if(!out)sprintf(output,"%s.cf",input_desc);
      computeCGDescriptors(image, desc);
    }else if(!strcmp(argv[i],"-sc")){
      if(!out)sprintf(output,"%s.sc",input_desc);
      computeShapeDescriptors(image, desc);
    }else if(!strcmp(argv[i],"-spin")){
      if(!out)sprintf(output,"%s.spin",input_desc);
      computeSpinDescriptors(image, desc);
    }else if(!strcmp(argv[i],"-pca")){
      if(!out)sprintf(output,"%s.pca",input_desc);
      computePcaDescriptors(image, desc);
    }else if(!strcmp(argv[i],"-DR")){
      dr=1;
    }else if(!strcmp(argv[i],"-c")){
      color=atoi(argv[i+1]);
    }
  }

  // Remove ones falling over the edge.
  for (size_t i=0; i<desc.size(); ) {
    if (!desc[i]->is_fully_inside(0, image->x(), 0, image->y())) {
      delete desc[i];
      desc.erase(desc.begin() + i);
    }
    else
      i++;
  }

  #ifndef QUIET
  cout << "time "<< tell_time(t)<< endl;
  cout << "output file: " << output<< endl;
  #endif
  if(out == 0)
    writeCorners(desc, output,1);
  else if(out==1)
    writeCorners(desc, output,1);
  else if(out==2)
    writeCorners(desc, output,0);
  else if(out==3)
    writeCorners(desc, output,2);

  if(dr){
    char draw[512];
    sprintf(draw,"%s.pgm",output);
    cout << "drawing points in: " << draw<< endl;
    drawAffineCorners(image, desc, draw,color);
  }

  for (size_t i=0; i<desc.size(); ++i)
      delete desc[i];

  delete image;

  return 0;
}

// avoid read/wrie to disk and share detected points in-memory
// updated by @Abhishek Dutta (29 Mar. 2017)
//
// precondition  : jpg_filename must exist and be accessible
// postcondition : regions contain the regions of interest
void compute_descriptors_sift(std::string jpg_filename,
                              std::vector<ellipse> &regions,
                              uint32_t& feat_count,
                              float scale_multiplier,
                              bool upright,
                              float *&descs ) {
  DARY *image = new ImageContent( jpg_filename.c_str() );
  image->toGRAY();
  image->char2float();
  vector< CornerDescriptor* > descriptors;

  // regions -> descriptors
  double x, y, a, b, c;
  Matrix U(2, 2, 0.0), D, Vi, V;
  CornerDescriptor* corneri;

  for ( unsigned int i=0; i < regions.size(); i++ ) {
    regions[i].get(x, y, a, b, c);

    U(1,1) = a;
    U(1,2) = b;
    U(2,1) = b;
    U(2,2) = c;
    U.svd(Vi,D,V);
    D(1,1) = ( 1.0 / sqrt(D(1,1)) );
    D(2,2) = ( 1.0 / sqrt(D(2,2)) );
    a = sqrt( D(2,2)*D(1,1) );

    D.tabMat[2][2] = D.tabMat[2][2] / a;
    D.tabMat[1][1] = D.tabMat[1][1] / a;
    U = V * D * V.transpose();

    corneri = new CornerDescriptor();
    // JAMES
    //c_scale=3.0*a;
    corneri->setCornerScale( scale_multiplier * a );
    corneri->setX_Y(x, y);
    corneri->setMi( U(1,1), U(1,2), U(2,1), U(2,2) );

    descriptors.push_back(corneri);
  }

  if (upright) {
    for(unsigned int i=0; i < descriptors.size(); i++){
      descriptors[i]->setAngle(0.0);
    }
  }

  computeSiftDescriptors(image, descriptors);

  // Remove ones falling over the edge.
  for (size_t i=0; i < descriptors.size(); ) {
    if (!descriptors[i]->is_fully_inside(0, image->x(), 0, image->y())) {
      delete descriptors[i];
      descriptors.erase(descriptors.begin() + i);
    }
    else {
      i++;
    }
  }

  feat_count = descriptors.size();
  uint32_t desc_dim = descriptors[0]->getSize();

  descs = new float[ feat_count * desc_dim ];
  float *desc_iter = descs;
  regions.resize( feat_count );

  for ( unsigned int i=0; i < feat_count; i++) {
    U(1,1) = descriptors[i]->getMi11();
    U(1,2) = descriptors[i]->getMi12();
    U(2,1) = descriptors[i]->getMi21();
    U(2,2) = descriptors[i]->getMi22();
    U.svd(Vi,D,V);

    D = D * descriptors[i]->getCornerScale();
    D(1,1) = 1.0 / ( D(1,1)*D(1,1) );
    D(2,2) = 1.0 / ( D(2,2)*D(2,2) );
    U = V * D * V.transpose();

    regions[i].set( descriptors[i]->getX(), descriptors[i]->getY(), U(1,1), U(1,2), U(2,2) );

    float* descriptors_i = descriptors[i]->getVec();
    for ( unsigned int j=0; j < desc_dim; ++j ) {
      *desc_iter = descriptors_i[j];
      desc_iter++;
    }
  }
}


} // end of namespace: KM_compute_descriptors

/*
int main(int argc, char **argv){
    int ret= KM_compute_descriptors::lib_main(argc,argv);
    if (ret!=0)
        exit(ret);
    return 0;
}
*/
