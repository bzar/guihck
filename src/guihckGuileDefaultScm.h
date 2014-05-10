#ifndef GUIHCKGUILEDEFAULTSCM_H
#define GUIHCKGUILEDEFAULTSCM_H

static const char GUIHCK_GUILE_DEFAULT_SCM[] =
"(define (create-elements . elements)"
"  (map (lambda (e) (e)) elements))"

"(define (create-element type props children)"
"  (define (set-properties! props)"
"    (if (null? props) '() (begin"
"      (set-element-property! (car props) (cadr props))"
"      (set-properties! (cddr props)))))"
"  (lambda ()"
"    (begin"
"      (push-new-element! type)"
"      (set-properties! props)"
"      (map (lambda (c) (c)) children)"
"      (if (list? (get-element-property 'onLoaded)) (primitive-eval (get-element-property 'onLoaded)) #t)"
"      (pop-element!))))"

"(define (composite constructor default-props . default-children)"
"  (lambda (props . children) "
"    (apply constructor (append (list (append default-props props)) default-children children))))"

"(define (with-parent)"
"  (lambda (func)"
"    (begin"
"      (push-parent-element!)"
"      (let ((value (func)))"
"        (pop-element!)"
"        value))))"

"(define (with-child index)"
"  (lambda (func)"
"    (begin"
"      (push-child-element! index)"
"      (let ((value (func)))"
"        (pop-element!)"
"        value))))"

"(define (with-children func)"
"  (let ((children (get-element-child-count)))"
"    (define (iter i)"
"      (if (>= i children)"
"        #t"
"        (begin"
"          ((with-child i) func)"
"          (iter (+ i 1)))))"
"    (iter 0)))"

"(define (get-parent-property prop)"
"  ((with-parent) (lambda ()"
"    (get-element-property prop))))"
;
#endif // GUIHCKGUILEDEFAULTSCM_H
