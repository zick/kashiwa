(load "./gen.scm")
(load "./cps.scm")
(load "./macro.scm")

(define (compile-define exp init global run-exps)
  (let ((name (cadr exp))
        (args (cddr exp)))
    (cond ((pair? name)
           (let ((cps-lambda
                  (cadr (cps (macroexpand (list* 'lambda (cdr name) args)))))
                 (clos (gensym "clos")))
             (gen cps-lambda)
             (push-function-vars! (cons 'lobject (car name)) global)
             (push-function-vars! (cons "cont_t*" clos) init)
             (push-function-body!
              (list
               (list clos " = malloc(sizeof(cont_t))")
               (list clos "->tag = TAG_CONT")
               (list clos "->env = NULL")
               (list clos "->fn = (function1_t)" (find-lambda-name cps-lambda))
               (list (car name) " = (lobject)" clos))
              init)))
          (else
           (push-function-vars! (cons 'lobject name) global)
           (push-function-body! (list "add_heap_rootset(&" name ")") init)
           (set-cdr! run-exps
                     (cons (list 'set! name (car args)) (cdr run-exps)))))))

(define (compile-exp exp init global run-exps)
  (cond ((not (pair? exp)) #f)
        ((eq? (car exp) 'define)
         (compile-define exp init global run-exps))
        (else
         (set-cdr! run-exps
                   (cons exp (cdr run-exps))))))

(define (write-header-file port)
  (display "#include \"builtin.h\"" port) (newline port)
  (display "#include \"function.h\"" port) (newline port)
  (display "#include \"symbol.h\"" port) (newline port)
  (newline port)
  (for-each
   (lambda (x)
     (display (string-append (translate-prototype (cdr x)) ";") port)
     (newline port))
   compiled-results))

(define (write-c-file port init global main end-of-toplevel-exp header)
  (display (string-append "#include \"" header "\"") port) (newline port)
  (newline port)
  (display "#include \"heap.h\"" port) (newline port)
  (display "#include \"stack.h\"" port) (newline port)
  (newline port)
  (display (string-append "#include <assert.h>") port) (newline port)
  (display (string-append "#include <setjmp.h>") port) (newline port)
  (display (string-append "#include <stdlib.h>") port) (newline port)
  (newline port)
  (display (translate-local-vars (function-vars global) 0) port)
  (newline port)
  (for-each
   (lambda (x)
     (display (translate-to-c (cdr x)) port) (newline port))
   compiled-results)
  (display (translate-to-c init) port)
  (newline port)
  (display (translate-to-c end-of-toplevel-exp) port)
  (newline port)
  (display (translate-to-c main 'int) port))

(define (make-end-of-toplevel-exp-function end-of-toplevel-exp)
  (set-function-name! end-of-toplevel-exp "end_of_toplevel_exp")
  (set-function-args! end-of-toplevel-exp '(("env_t*" env) (lobject x)))
  (set-function-vars! end-of-toplevel-exp '((cont_t c)))
  (push-function-body!
   (list 'if "++toplevel_exps_index < num_toplevel_exps"
         (list
          (list "c.tag = TAG_CONT")
          (list "c.env = NULL")
          (list "c.fn = end_of_toplevel_exp")
          (list "toplevel_exps[toplevel_exps_index](NULL, (lobject)&c)"))
         (list "return"))
   end-of-toplevel-exp))

(define (make-main-function run-exps main end-of-toplevel-exp entry-point)
  (set-function-name! main "main")
  (set-function-args! main '((int . argc) ("char**" argv)))
  (push-function-body!
   (list
    (list "stack_bottom = (char*)&" (cdar (function-args main)))
    (list "entry_point = &" entry-point)
    (list "init_heap()")
    (list "init_symbol()")
    (list "init_builtin()")
    (list "init()")
    (list "num_toplevel_exps = " (length run-exps))
    (list "toplevel_exps_index = 0"))
   main)
  (do ((i 0 (+ i 1))
       (exps (reverse run-exps) (cdr exps)))
      ((null? exps) #t)
    (let ((lambda-exp
           (cadr (cps (macroexpand (list 'lambda '() (car exps)))))))
      (gen lambda-exp)
      (push-function-body!
       (list "toplevel_exps[" i "] = (function1_t)"
             (find-lambda-name lambda-exp))
       main)))
  (let ((cont (gensym "cont")))
    (push-function-vars! (cons 'cont_t cont) main)
    (push-function-body!
     (list (list cont ".tag = TAG_CONT")
           (list cont ".env = NULL")
           (list cont ".fn = " (function-name end-of-toplevel-exp))
           (list 'if "!setjmp(*entry_point)"
                 (list "toplevel_exps[0](NULL, (lobject)&" cont ")")
                 (list "CALL_THUNK(restart_thunk)"))
           (list "return 0"))
     main)))

(define (compile-file input output)
  (init-compile)
  (let ((init (make-function))
        (run-exps (cons 'dummy '()))
        (global (make-function))
        (main (make-function))
        (end-of-toplevel-exp (make-function))
        (entry-point (gensym "entry_point")))
    (set-function-name! init "init")
    (set-function-vars!
     global
     (list (cons "static int" 'num_toplevel_exps)
           (cons "static int" 'toplevel_exps_index)
           (cons "static jmp_buf" entry-point)))
    (call-with-input-file input
      (lambda (inp)
        (do ((exp (read inp) (read inp)))
            ((eof-object? exp) #t)
          (compile-exp exp init global run-exps))))

    (push-function-vars!
     (cons "static function1_t"
           (string-append "toplevel_exps["
                          (number->string (length (cdr run-exps)))
                          "]"))
     global)
    (make-end-of-toplevel-exp-function end-of-toplevel-exp)
    (make-main-function (cdr run-exps) main end-of-toplevel-exp entry-point)
    (call-with-output-file (string-append output ".h")
      write-header-file)
    (call-with-output-file (string-append output ".c")
      (lambda (port)
        (write-c-file port init global main end-of-toplevel-exp
                      (string-append output ".h"))))))