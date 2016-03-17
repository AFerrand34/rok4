# Copyright © (2011) Institut national de l'information
#                    géographique et forestière
#
# Géoportail SAV <geop_services@geoportail.fr>
#
# This software is a computer program whose purpose is to publish geographic
# data using OGC WMS and WMTS protocol.
#
# This software is governed by the CeCILL-C license under French law and
# abiding by the rules of distribution of free software.  You can  use,
# modify and/ or redistribute the software under the terms of the CeCILL-C
# license as circulated by CEA, CNRS and INRIA at the following URL
# "http://www.cecill.info".
#
# As a counterpart to the access to the source code and  rights to copy,
# modify and redistribute granted by the license, users are provided only
# with a limited warranty  and the software's author,  the holder of the
# economic rights,  and the successive licensors  have only  limited
# liability.
#
# In this respect, the user's attention is drawn to the risks associated
# with loading,  using,  modifying and/or developing or reproducing the
# software by the user in light of its specific status of free software,
# that may mean  that it is complicated to manipulate,  and  that  also
# therefore means  that it is reserved for developers  and  experienced
# professionals having in-depth computer knowledge. Users are therefore
# encouraged to load and test the software's suitability as regards their
# requirements in conditions enabling the security of their systems and/or
# data to be ensured and,  more generally, to use and operate it in the
# same conditions as regards security.
#
# The fact that you are presently reading this means that you have had
#
# knowledge of the CeCILL-C license and that you accept its terms.
################################################################################

use strict;
use warnings;

use FindBin qw($Bin);

use Test::More;
use Data::Dumper;

use Log::Log4perl qw(:easy);
# logger by default for unit tests
Log::Log4perl->easy_init({
    level => $WARN,
    layout => '%5p : %m (%M) %n'
});

# My tested class
use WMTSALAD::Pyramid;


my $valid_prop = 'be4/pyramide/tests/properties/WMTSalaD_valid_prop.conf';
my $valid_src = 'be4/pyramide/tests/sources/WMTSalaD_valid_src.txt';

my $prop_buffer; # buffer to store the properties file's content
my $prop_fh; # file handle for this file
my $temp_prop_file = 'be4/pyramide/tests/properties/WMTSalaD_temp_prop.conf'; # temporary properties file for tests
my $src_buffer; # buffer to store the datasources file's content
my $src_fh; # file handle for this file
my $temp_src_file = 'be4/pyramide/tests/sources/WMTSalaD_temp_src.txt'; # temporary datasources file for tests

open ($prop_fh, '<', $valid_prop) or die ("Unable to open properties file.");
$prop_buffer = do { local $/; <$prop_fh> };
close ($prop_fh);

####################### Good ######################

# Pyramid creation with all parameters defined
my $pyramid = WMTSALAD::Pyramid->new($valid_prop, $valid_src);
if (defined $pyramid) { print(sprintf "Pyramid object content : %s", $pyramid->dumpPyrHash()); }
ok (defined $pyramid, "Pyramid created (exhaustive parameters)");
undef $pyramid;

# Pyramid creation with optionnal parameters undefined
$prop_buffer =~ s/\n(persistent)/\n;$1/;
$prop_buffer =~ s/\n(dir_mask)/\n;$1/;
$prop_buffer =~ s/\n(dir_metadata)/\n;$1/;
$prop_buffer =~ s/\n(compression)/\n;$1/;
$prop_buffer =~ s/\n(photometric)/\n;$1/;
$prop_buffer =~ s/\n(interpolation)/\n;$1/;
$prop_buffer =~ s/\n(color)/\n;$1/;
my $written = writeTemp($prop_buffer, $temp_prop_file);
$prop_buffer =~ s/\n;/\n/g;
$pyramid = WMTSALAD::Pyramid->new($temp_prop_file, $valid_src);
if (defined $pyramid) { print(sprintf "Pyramid object content : %s", $pyramid->dumpPyrHash()); }
ok (defined $pyramid, "Pyramid created (only mandatory parameters)");
undef $pyramid;

####################### Bad #######################

# Pyramid creation with no properties configuration file
my $errPyramid = WMTSALAD::Pyramid->new();
ok(! defined $errPyramid, "Pyramid with no properties configuration file");
undef $errPyramid;

# Properties file error : invalid 'persistent' value
$prop_buffer =~ s/(persistent =) true/$1 ThisIsNoBoolean/;
$written = writeTemp($prop_buffer, $temp_prop_file);
$prop_buffer =~ s/ThisIsNoBoolean/true/;
$errPyramid = WMTSALAD::Pyramid->new($temp_prop_file, $valid_src);
ok(! defined $errPyramid, "Properties file error : invalid 'persistent' value");
undef $errPyramid;

# Properties file error : unnamed pyramid
$prop_buffer =~ s/\n(pyr_name)/\n;$1/;
$written = writeTemp($prop_buffer, $temp_prop_file);
$prop_buffer =~ s/\n;(pyr_name)/\n$1/;
$errPyramid = WMTSALAD::Pyramid->new($temp_prop_file, $valid_src);
ok(! defined $errPyramid, "Properties file error : unnamed pyramid");
undef $errPyramid;

# Properties file error : no data path
$prop_buffer =~ s/\n(pyr_data_path)/\n;$1/;
$written = writeTemp($prop_buffer, $temp_prop_file);
$prop_buffer =~ s/\n;(pyr_data_path)/\n$1/;
$errPyramid = WMTSALAD::Pyramid->new($temp_prop_file, $valid_src);
ok(! defined $errPyramid, "Properties file error : no data path");
undef $errPyramid;

# Properties file error : no descriptor path
$prop_buffer =~ s/\n(pyr_desc_path)/\n;$1/;
$written = writeTemp($prop_buffer, $temp_prop_file);
$prop_buffer =~ s/\n;(pyr_desc_path)/\n$1/;
$errPyramid = WMTSALAD::Pyramid->new($temp_prop_file, $valid_src);
ok(! defined $errPyramid, "Properties file error : no descriptor path");
undef $errPyramid;

# Properties file error : no TMS name
$prop_buffer =~ s/\n(tms_name)/\n;$1/;
$written = writeTemp($prop_buffer, $temp_prop_file);
$prop_buffer =~ s/\n;(tms_name)/\n$1/;
$errPyramid = WMTSALAD::Pyramid->new($temp_prop_file, $valid_src);
ok(! defined $errPyramid, "Properties file error : no TMS name");
undef $errPyramid;

# Properties file error : no TMS path
$prop_buffer =~ s/\n(tms_path)/\n;$1/;
$written = writeTemp($prop_buffer, $temp_prop_file);
$prop_buffer =~ s/\n;(tms_path)/\n$1/;
$errPyramid = WMTSALAD::Pyramid->new($temp_prop_file, $valid_src);
ok(! defined $errPyramid, "Properties file error : no TMS path");
undef $errPyramid;

# Properties file error : no image_width
$prop_buffer =~ s/\n(image_width)/\n;$1/;
$written = writeTemp($prop_buffer, $temp_prop_file);
$prop_buffer =~ s/\n;(image_width)/\n$1/;
$errPyramid = WMTSALAD::Pyramid->new($temp_prop_file, $valid_src);
ok(! defined $errPyramid, "Properties file error : no image_width");
undef $errPyramid;

# Properties file error : no image_height
$prop_buffer =~ s/\n(image_height)/\n;$1/;
$written = writeTemp($prop_buffer, $temp_prop_file);
$prop_buffer =~ s/\n;(image_height)/\n$1/;
$errPyramid = WMTSALAD::Pyramid->new($temp_prop_file, $valid_src);
ok(! defined $errPyramid, "Properties file error : no image_height");
undef $errPyramid;

# Properties file error : invalid image_height
$prop_buffer =~ s/(image_height =) 16/$1 -0/;
$written = writeTemp($prop_buffer, $temp_prop_file);
$prop_buffer =~ s/(image_height =) -0/$1 16/;
$errPyramid = WMTSALAD::Pyramid->new($temp_prop_file, $valid_src);
ok(! defined $errPyramid, "Properties file error : invalid image_height");
undef $errPyramid;

# Properties file error : no dir_depth
$prop_buffer =~ s/\n(dir_depth)/\n;$1/;
$written = writeTemp($prop_buffer, $temp_prop_file);
$prop_buffer =~ s/\n;(dir_depth)/\n$1/;
$errPyramid = WMTSALAD::Pyramid->new($temp_prop_file, $valid_src);
ok(! defined $errPyramid, "Properties file error : no dir_depth");
undef $errPyramid;

# Properties file error : invalid dir_depth
$prop_buffer =~ s/(dir_depth =) 2/$1 -0/;
$written = writeTemp($prop_buffer, $temp_prop_file);
$prop_buffer =~ s/(dir_depth =) -0/$1 2/;
$errPyramid = WMTSALAD::Pyramid->new($temp_prop_file, $valid_src);
ok(! defined $errPyramid, "Properties file error : invalid dir_depth");
undef $errPyramid;

# Properties file error : no dir_image
$prop_buffer =~ s/\n(dir_image)/\n;$1/;
$written = writeTemp($prop_buffer, $temp_prop_file);
$prop_buffer =~ s/\n;(dir_image)/\n$1/;
$errPyramid = WMTSALAD::Pyramid->new($temp_prop_file, $valid_src);
ok(! defined $errPyramid, "Properties file error : no dir_image");
undef $errPyramid;

# Properties file error : no dir_nodata
$prop_buffer =~ s/\n(dir_nodata)/\n;$1/;
$written = writeTemp($prop_buffer, $temp_prop_file);
$prop_buffer =~ s/\n;(dir_nodata)/\n$1/;
$errPyramid = WMTSALAD::Pyramid->new($temp_prop_file, $valid_src);
ok(! defined $errPyramid, "Properties file error : no dir_nodata");
undef $errPyramid;

# Properties file error : no bitspersample
$prop_buffer =~ s/\n(bitspersample)/\n;$1/;
$written = writeTemp($prop_buffer, $temp_prop_file);
$prop_buffer =~ s/\n;(bitspersample)/\n$1/;
$errPyramid = WMTSALAD::Pyramid->new($temp_prop_file, $valid_src);
ok(! defined $errPyramid, "Properties file error : no bitspersample");
undef $errPyramid;

# Properties file error : invalid bitspersample
$prop_buffer =~ s/(bitspersample =) 8/$1 -0/;
$written = writeTemp($prop_buffer, $temp_prop_file);
$prop_buffer =~ s/(bitspersample =) -0/$1 8/;
$errPyramid = WMTSALAD::Pyramid->new($temp_prop_file, $valid_src);
ok(! defined $errPyramid, "Properties file error : invalid bitspersample");
undef $errPyramid;

# Properties file error : no sampleformat
$prop_buffer =~ s/\n(sampleformat)/\n;$1/;
$written = writeTemp($prop_buffer, $temp_prop_file);
$prop_buffer =~ s/\n;(sampleformat)/\n$1/;
$errPyramid = WMTSALAD::Pyramid->new($temp_prop_file, $valid_src);
ok(! defined $errPyramid, "Properties file error : no sampleformat");
undef $errPyramid;

# Properties file error : invalid sampleformat
$prop_buffer =~ s/(sampleformat =) int/$1 complex/;
$written = writeTemp($prop_buffer, $temp_prop_file);
$prop_buffer =~ s/(sampleformat =) complex/$1 int/;
$errPyramid = WMTSALAD::Pyramid->new($temp_prop_file, $valid_src);
ok(! defined $errPyramid, "Properties file error : invalid sampleformat");
undef $errPyramid;

# Properties file error : no samplesperpixel
$prop_buffer =~ s/\n(samplesperpixel)/\n;$1/;
$written = writeTemp($prop_buffer, $temp_prop_file);
$prop_buffer =~ s/\n;(samplesperpixel)/\n$1/;
$errPyramid = WMTSALAD::Pyramid->new($temp_prop_file, $valid_src);
ok(! defined $errPyramid, "Properties file error : no samplesperpixel");
undef $errPyramid;

# Properties file error : invalid samplesperpixel
$prop_buffer =~ s/(samplesperpixel =) 3/$1 -0/;
$written = writeTemp($prop_buffer, $temp_prop_file);
$prop_buffer =~ s/(samplesperpixel =) -0/$1 3/;
$errPyramid = WMTSALAD::Pyramid->new($temp_prop_file, $valid_src);
ok(! defined $errPyramid, "Properties file error : invalid samplesperpixel");
undef $errPyramid;

# Properties file error : invalid photometric
$prop_buffer =~ s/(photometric =) rgb/$1 complex/;
$written = writeTemp($prop_buffer, $temp_prop_file);
$prop_buffer =~ s/(photometric =) complex/$1 rgb/;
$errPyramid = WMTSALAD::Pyramid->new($temp_prop_file, $valid_src);
ok(! defined $errPyramid, "Properties file error : invalid photometric");
undef $errPyramid;

# Properties file error : invalid interpolation
$prop_buffer =~ s/(interpolation =) bicubic/$1 complex/;
$written = writeTemp($prop_buffer, $temp_prop_file);
$prop_buffer =~ s/(interpolation =) complex/$1 bicubic/;
$errPyramid = WMTSALAD::Pyramid->new($temp_prop_file, $valid_src);
ok(! defined $errPyramid, "Properties file error : invalid interpolation");
undef $errPyramid;

# Properties file error : invalid color
$prop_buffer =~ s/(color =) 255,255,255/$1 256,0/;
$written = writeTemp($prop_buffer, $temp_prop_file);
$prop_buffer =~ s/(color =) 256,0/$1 255,255,255/;
$errPyramid = WMTSALAD::Pyramid->new($temp_prop_file, $valid_src);
ok(! defined $errPyramid, "Properties file error : invalid color");
undef $errPyramid;


####################### End #######################

sub writeTemp {
    my $content = shift;
    my $file = shift;

    open (my $handle, ">", $file) or return 0;
    $handle->print($content);
    close ($handle) or return 0;

    return 1;
}


done_testing();
