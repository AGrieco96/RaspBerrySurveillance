$(function(){

  function move(angle){
    $.post('./move.php' , {src : angle} , function(){
    });
  };

  $("#angle").val($("#myRange").val());

  $("#myRange").on("input" , function(){
    $("#angle").val($("#myRange").val());
  }); 

  $("#myRange").on("mouseup" , function(){
    //alert("daje");
    //funzione sposta motore
    move($("#myRange").val());
  });

  $("#angle").on("keyup" , function(){
    if (event.which == 13) {
    //controllo value
      if($("#angle").val() >= -90 && $("#angle").val() <= 90){
        $("#myRange").val($("#angle").val());
        //funzione sposta motore
        move($("#myRange").val());
      }
      else{
        //output.value = slider.value;
        window.alert("Out of Range\nMin:-90° Max:90°");
        $("#angle").val($("#myRange").val());
      }
    }
  });

  $("#photo").on('click' , function(){
   $.get('./photo.php', function(){
	$("#refresh").attr("src", "http://raspberrypi.local:8080");
   });
  });


});



