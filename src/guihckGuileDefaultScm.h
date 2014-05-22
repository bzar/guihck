#ifndef GUIHCKGUILEDEFAULTSCM_H
#define GUIHCKGUILEDEFAULTSCM_H

static const char GUIHCK_GUILE_DEFAULT_SCM[] =

    "(define (flatmap f xs) (apply append (map f xs)))"

    "(define (create-elements! . elements)"
    "  (map (lambda (e) (e)) elements))"

    "(define (set-properties! pred props)"
    "  (if (null? props) '()"
    "    (let ((key (car props)) (value (cadr props)) (rest (cddr props)))"
    "      (if (pred key value)"
    "        (set-element-property! key value))"
    "      (set-properties! pred rest))))"

    "(define (create-element type props children)"
    "  (lambda ()"
    "    (push-new-element! type)"
    "    (set-properties! (lambda (k v) (eq? k 'id)) props)"
    "    (map (lambda (c) (c)) children)"
    "    (set-properties! (lambda (k v) (not (eq? k 'id))) props)"
    "    (if (list? (get-element-property 'init))"
    "      (primitive-eval (get-element-property 'init)))"
    "    (pop-element!)))"

    "(define (composite constructor default-props . default-children)"
    "  (lambda (props . children)"
    "    (apply constructor (cons (append default-props props) (append default-children children)))))"

    "(define set-prop!"
    "  (case-lambda"
    "    ((property value) (set-prop! (get-element) property value))"
    "    ((element property value)"
    "      (push-element! element)"
    "      (set-element-property! property value)"
    "      (pop-element!))))"

    "(define get-prop"
    "  (case-lambda"
    "    ((property) (get-prop (get-element) property))"
    "    ((element property)"
    "      (push-element! element)"
    "      (let ((value (get-element-property property)))"
    "        (pop-element!)"
    "        value))))"

    "(define this get-element)"

    "(define (find-element id)"
    "  (begin"
    "    (push-element-by-id! id)"
    "    (let ((result-element (get-element)))"
    "      (pop-element!)"
    "      result-element)))"

    "(define parent"
    "  (case-lambda"
    "    (() (parent (get-element)))"
    "    ((element)"
    "      (push-element! element)"
    "      (push-parent-element!)"
    "      (let ((parent-element (get-element)))"
    "        (pop-element!)"
    "        (pop-element!)"
    "        parent-element))))"


    "(define child"
    "  (case-lambda"
    "    ((index) (child (get-element) index))"
    "    ((element index)"
    "      (push-element! element)"
    "      (push-child-element! index)"
    "      (let ((child-element (get-element)))"
    "        (pop-element!)"
    "        (pop-element!)"
    "        child-element))))"

    "(define children"
    "  (case-lambda"
    "    (() (children (get-element)))"
    "    ((element)"
    "      (push-element! element)"
    "      (let ((n (get-element-child-count)))"
    "        (define (iter i lst)"
    "          (if (< i n)"
    "            (begin"
    "              (push-child-element! i)"
    "              (let ((c (get-element)))"
    "                (pop-element!)"
    "                (iter (+ i 1) (cons c lst))))"
    "            lst))"
    "        (let ((result (reverse! (iter 0 '()))))"
    "          (pop-element!)"
    "          result)))))"

    "(define (observe . vals)"
    "  (define (resolve e)"
    "    (cond ((eq? e 'parent) (parent))"
    "          ((eq? e 'this) (this))"
    "          (else (find-element e))))"
    "  (define (iter lst result)"
    "    (if (null? lst)"
    "      result"
    "      (iter (cddr lst) "
    "            (cons (cons (resolve (car lst)) (cadr lst)) result))))"
    "  (iter vals '()))"

    "(define bind"
    "  (case-lambda"
    "    ((property callback) (bind (this) property callback))"
    "    ((element property callback)"
    "      (add-element-property-listener! element property callback))))"

    "(define unbind remove-element-property-listener!)"
    ;
#endif // GUIHCKGUILEDEFAULTSCM_H
