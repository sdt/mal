#!/usr/bin/env perl6

use v6;
use Core;
use Printer;
use Reader;
use ReadLine;
use Types;

my @library =
    '(def! not (fn* (a) (if a false true)))',
    '(def! load-file (fn* (f) (eval (read-string (str "(do " (slurp f) ")")))))',
    q{(defmacro! cond (fn* (& xs) (if (> (count xs) 0) (list 'if (first xs) (if (> (count xs) 1) (nth xs 1) (throw "odd number of forms to cond")) (cons 'cond (rest (rest xs)))))))},
    q{(defmacro! or (fn* (& xs) (if (empty? xs) nil (if (= 1 (count xs)) (first xs) `(let* (or_FIXME ~(first xs)) (if or_FIXME or_FIXME (or ~@(rest xs))))))))},
    ;

# This gets called when there is no argv
multi sub MAIN() {
    my $repl-env = setup-env([]);

    while defined (my $input = read-line('user> ')) {
        say rep($input, $repl-env);
    }
}

# This gets called when there is an argv, and the first arg is a file
multi sub MAIN(Str $filename, *@args) {
    my $repl-env = setup-env(@args);

    rep("(load-file \"$filename\")", $repl-env);
}

sub setup-env(@argv) {
    my $env = malEnv.new;
    install-core($env, :eval({ malEVAL($_, $env) }));

    for @library { rep($_, $env) }

    # Install argv
    $env.set('*ARGV*', malList.new(@argv.map: { malString.new($_) }));

    return $env;
}

sub rep(Str $input, $env) {
    return malPRINT(malEVAL(malREAD($input), $env));

    CATCH {
        when Reader::EmptyInput {
            # nothing
        }
        default {
            say %*ENV<MAL_DEBUG>:exists ?? $_ !! $_.message;
        }
    }
}

sub malREAD(Str $input) {
    return read-str($input);
}

sub malEVAL(malValue $ast is copy, malEnv $env is copy) {

    my %special =
        'def!' => sub (malSymbol $sym, malValue $def) {
            return $env.set($sym.value, malEVAL($def, $env));
        },
        'defmacro!' => sub (malSymbol $sym, malValue $def) {
            my $macro = malMacro.new(malEVAL($def, $env));
            return $env.set($sym.value, $macro);
        },
        'fn*' => sub (malSequence $args, malValue $body) {
            return malLambda.new($args, $body, $env);
        },
        'macroexpand' => sub (malValue $ast) {
            return macro-expand($ast, $env);
        },
        'quote' => sub (malValue $quoted) {
            return $quoted;
        },
        ;

    my %special-tailrec =
        'do' => sub (*@values) {
            my $ret;
            my $n = @values.elems;
            for @values[^($n-1)] -> $value {
                malEVAL($value, $env);
            }
            return (@values[$n-1], $env);
        },
        'if' => sub (malValue $cond, malValue $then, malValue $else = malNil) {
            my $ast = is-true(malEVAL($cond, $env)) ?? $then !! $else;
            return ($ast, $env);
        },
        'let*' => sub (malSequence $bindings, malValue $expr) {
            my $inner = malEnv.new(:outer($env));
            for $bindings.value.list -> malSymbol $symbol, malValue $value {
                $inner.set($symbol.value, malEVAL($value, $inner));
            }
            return ($expr, $inner);
        },
        'quasiquote' => sub (malValue $quoted) {
            return (quasiquote($quoted), $env);
        },
        ;

    loop {
        unless ($ast ~~ malList) && ($ast.value.elems > 0) {
            return eval-ast($ast, $env);
        }

        $ast = macro-expand($ast, $env);
        unless ($ast ~~ malList) && ($ast.value.elems > 0) {
            return $ast;
        }

        my ($op, @args) = $ast.value.list;
        if $op ~~ malSymbol && %special{$op.value} -> $handler {
            return $handler(|@args);
            CATCH {
                die RuntimeError.new($op.value ~ ': ' ~ $_);
            }
        }
        elsif $op ~~ malSymbol && %special-tailrec{$op.value} -> $handler {
            ($ast, $env) = $handler(|@args);
            next; # TCO - continue the loop
            CATCH {
                die RuntimeError.new($op.value ~ ': ' ~ $_);
            }
        }
        else {
            my ($op, @args) = eval-ast($ast, $env).value.list;
            if $op ~~ malBuiltIn {
                return $op.value.(|@args);
            }
            if $op ~~ malLambda {
                my $inner = malEnv.new(:outer($op.env));
                $inner.bind($op.args, @args);
                ($ast, $env) = ($op.value, $inner);
                next; # TCO - continue the loop
            }
            die RuntimeError.new(pr-str($op, True) ~ " is not applicable");
        }
    }
}

sub malPRINT($ast) {
    return pr-str($ast, True);
}

sub eval-ast(malValue $ast, malEnv $env) {
    given ($ast) {
        when malSymbol {
            return $env.get($ast.value);
        }
        when malHash {
            my %value = $ast.value.pairs.map(
                { $_.key => malEVAL($_.value, $env) });
            return malHash.new(%value);
        }
        when malList {
            my @value = $ast.value.map({ malEVAL($_, $env) });
            return malList.new(@value);
        }
        when malVector {
            my @value = $ast.value.map({ malEVAL($_, $env) });
            return malVector.new(@value);
        }
        default {
            return $ast;
        }
    }
}

sub env-get(malEnv $env, malSymbol $symbol)
{
    my $owner = $env.find($symbol.value);
    return $owner ~~ malNil ?? $owner !! $owner.get($symbol.value);
}

sub is-macro-call(malValue $ast, malEnv $env)
{
    return is-pair($ast)
        && ($ast.value[0] ~~ malSymbol)
        && (env-get($env, $ast.value[0]) ~~ malMacro);
}

sub is-pair(malValue $ast) {
    return $ast ~~ malSequence && $ast.value.elems > 0;
}

sub is-symbol(malValue $ast, Str $symbol) {
    return $ast ~~ malSymbol && $ast.value ~~ $symbol;
}

sub macro-expand(malValue $ast is copy, malEnv $env) {
    while is-macro-call($ast, $env) {
        my ($symbol, @args) = $ast.value.list;
        my $lambda = $env.get($symbol.value).value;

        my $inner = malEnv.new(:outer($lambda.env));
        $inner.bind($lambda.args, @args);

        $ast = malEVAL($lambda.value, $inner);
    }
    return $ast;
}

sub quasiquote(malValue $ast) {
    if ! is-pair($ast) {
        return make-form('quote', $ast);
    }

    my ($op, @args) = $ast.value.list;
    if is-symbol($op, 'unquote') {
        if @args.elems != 1 {
            die RuntimeError.new('unquote requires exactly one arg');
        }
        return @args[0];
    }

    my $rest = malList.new(@args);
    if is-pair($op) && is-symbol($op.value[0], 'splice-unquote') {
        return make-form('concat', $op.value[1], quasiquote($rest));
    }
    return make-form('cons', quasiquote($op), quasiquote($rest));
}
