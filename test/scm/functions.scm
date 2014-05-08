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
  
(define (color-button props)
  (rectangle props
    (mouse-area (append props '(onMouseDown (set-clear-color (get-element-property! 'color)))))))
      
(create-elements
  (color-button '(color (255 0 0) x 100 y 100 width 50 height 50))
  (color-button '(color (0 255 0) x 200 y 100 width 50 height 50))
  (color-button '(color (0 0 255) x 300 y 100 width 50 height 50)))
                  
    
