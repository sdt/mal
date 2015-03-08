package MAL::Object;
use 5.20.0;
use warnings;

use Function::Parameters qw( :strict );
use Module::Load qw( load );
use Sub::Name qw( subname );

use overload '""' => method { $self->to_string };

{
    my @constants = qw( False Nil True );
    my @scalars   = qw( Integer Keyword String Symbol );
    my @compounds = qw( BuiltIn Hash Lambda Pair Vector );
    my @types     = (@constants, @scalars, @compounds);

    fun pkg($sym, $pkg = __PACKAGE__) {
        return $pkg . '::' . $sym;
    }

    fun make_method($method, $subref, $pkg = __PACKAGE__) {
        my $sym = pkg($method, $pkg);

        no strict 'refs';
        no warnings 'redefine';
        *{ $sym } = subname($sym => $subref);
    }

    # First of all, load up the subclass modules
    load pkg($_) for @types;

    # Now build constant constructors for all the constants
    for my $type (@constants) {
        my $pkg = pkg($type);
        my $value = $pkg->new;
        make_method(lc($type), sub { return $value });
    }

    # And regular constructors for all the rest
    for my $type (@scalars, @compounds) {
        my $module = pkg($type);
        make_method(lc($type), sub { shift; $module->new(@_) });
    }

    # Add in type detection methods
    for my $type (@types) {
        my $lctype = lc($type);
        my $method = "is_$lctype";
        make_method($method, sub { return });
        make_method($method, sub { return 1 }, pkg($type));
        make_method('type', sub { return $lctype }, pkg($type));
    }
}

method in_parens {
    return '(' . join(' ', @_) . ')';
}

method dump {
    use Data::Dumper::Concise; print STDERR Dumper($self);
}

method list($class: @items) {
    my $list = $class->nil;
    for my $item (reverse @items) {
        $list = $class->pair($item, $list);
    }
    return $list;
}

1;
