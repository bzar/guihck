(define (set-prop! id prop value)
  (begin
    (push-element! id)
    (set-element-property! prop value)
    (pop-element!)))
    
(create-elements
  (rectangle '(id button x 300 y 100 width 200 height 200)
    (mouse-area '(x 0 y 0 width 200 height 200 
                  color '(0 0 255)
                  onMouseDown (set-prop! 'button 'color '(0 0 255))
                  onMouseUp (set-prop! 'button 'color '(255 0 255))
                  onMouseMove (set-prop! 'button 'color '(255 0 0))
                  onMouseEnter (set-prop! 'button 'color '(255 0 0))
                  onMouseExit (set-prop! 'button 'color '(0 255 0))
                  ))))
    