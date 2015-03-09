package MAL::Object::Nil;
use 5.20.0;
use warnings;

use parent qw( MAL::Object::Constant );
use Function::Parameters qw( :strict );

method to_string {
    return 'nil';
}

1;
