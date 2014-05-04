(create-element! 'rectangle)
(set-element-property! 'x 200.0)
(set-element-property! 'y 100.0)
(set-element-property! 'width 50.0)
(set-element-property! 'height 75.0)
(set-element-property! 'color '(128 52 200))
(pop-element!)



(define (create-rectangle! x y w h c) 
  (begin 
    (create-element! 'rectangle)
    (set-element-property! 'x x)
    (set-element-property! 'y y)
    (set-element-property! 'width w)
    (set-element-property! 'height h)
    (set-element-property! 'color c)
    (pop-element!)))
   
(create-rectangle! 50 50 50 50 '(255 0 255))


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

(create-elements 
  (rectangle '(x 400
               y 400
               width 25
               height 25)
    (rectangle '(x 410
                 y 410
                 width 10
                 height 10
                 color (255 0 0)))
    (rectangle '(x 415
                 y 415
                 width 10
                 height 10
                 color (0 255 0))))
  (rectangle '(x 700
               y 100
               width 25
               height 25
               color (0 0 255))))