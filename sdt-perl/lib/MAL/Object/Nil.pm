package MAL::Object::Nil;
use 5.20.0;
use warnings;

use parent qw( MAL::Object::Constant );
use Function::Parameters qw( :strict );

method items {
    return ();
}

method to_string {
    return '()';
}

1;
