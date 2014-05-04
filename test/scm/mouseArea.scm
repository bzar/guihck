(define (create-elements . elements)
  (map (lambda (e) (e)) elements))
  
(define (create-element-with-properties type props . children)
  (define (set-properties! props)
    (if (null? props) '() (begin
      (set-element-property! (car props) (cadr props))
      (set-properties! (cddr props)))))
    
  (lambda () 
    (begin
      (create-element! type)
      (set-properties! props)
      (map (lambda (c) (c)) children)
      (pop-element!))))

(define (rectangle props . children) 
  (apply create-element-with-properties (append (list 'rectangle props) children)))
  
(define (mouse-area props . children) 
  (apply create-element-with-properties (append (list 'mouse-area props) children)))
  
  
(create-elements
  (rectangle '(x 300 y 100 width 200 height 200)
    (mouse-area '(x 300 y 100 width 200 height 200))))