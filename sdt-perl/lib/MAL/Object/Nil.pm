package MAL::Object::Nil;
use 5.20.0;
use warnings;

use Moo;
with qw( MAL::Interface::Object );

use Function::Parameters qw( :strict );

method to_string {
    return '()';
}

1;
