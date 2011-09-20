(define num-gensym 0)

(define (gensym . opt)
  (let ((prefix (if (null? opt) "g" (car opt))))
    (set! num-gensym (+ num-gensym 1))
    (string->symbol
     (string-append prefix (number->string num-gensym) "_"))))

(define (ensure-string x)
  (cond ((null? x) "")
        ((symbol? x) (symbol->string x))
        ((number? x) (number->string x))
        ((pair? x) (string-append (ensure-string (car x))
                                  (ensure-string (cdr x))))
        (else x)))

(define (position x lst)
  (define (iter lst n)
    (cond ((null? lst) #f)
          ((eq? (car lst) x) n)
          (else (iter (cdr lst) (+ n 1)))))
  (iter lst 0))

(define (repeat-string n str)
  (define (iter n acc)
    (if (= n 0)
        acc
        (iter (- n 1) (string-append str acc))))
  (iter n ""))

(define (implode glue lst)
  (cond ((null? lst) lst)
        ((null? (cdr lst)) lst)
        (else (list* (car lst) glue (implode glue (cdr lst))))))

(define (copy-tree tree)
  (if (pair? tree)
      (cons (copy-tree (car tree)) (copy-tree (cdr tree)))
      tree))

(define builtin-list
  '(
    eqv? eq? number? complex? real? rational? integer? exact? inexact?
         = < > <= >= + * - / quotient remainder modulo numerator denominator
         floor ceiling truncate round exp log sin cons tan asin acos tan asin acos
         atan sqrt expt make-rectangular make-polar real-part imag-part magnitude
         angle exact->inexact inexact->exact number->string string->number pair? cons
         car cdr set-car! set-cdr! caar cadr cdar cddr symbol? symbol->string
         string->symbol char? char=? char<? char>? char<=? char>=? char->integer
         integer->char string? make-string string-length string-ref string-set!
         vector? make-vector vector-length vector-ref vector-set! procedure? apply
         call-with-current-continuation values call-with-values dynamic-wind
         eval scheme-report-environment null-environment input-port?
         output-port? current-input-port current-output-port open-input-file
         open-output-file close-input-port close-output-port read peek-char
         eof-object? char-ready?

         equal? zero? positive? negative? odd? even? max min abs gcd lcm not boolean?
         null? list? list length append reverse list-tail list-ref memq memv member
         assq assv assoc char-ci=? char-ci<? char-ci>? char-ci<=? char-ci>=?
         char-alphabetic? char-numeric? char-whitespace? char-upper-case?
         char-lower-case? char-upcase char-downcase string string=? string-ci=?
         string<? string>? string<=? string>=? string-ci<? string-ci>? string-ci<=?
         string-ci>=? substring string-append string->list list->string string-copy
         string-fill! vector vector->list list->vector vector-fill! map for-each
         force delay call-with-input-file call-with-output-file read write display
         newline write-char

         interaction-environment with-input-from-file with-output-to-file load
         transcript-on transcript-off
         ))
