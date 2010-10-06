var filteredDocs;
var master;

$(document).ready(function(){

  $('#filterTab').click( function() { 
	$('#filterOptions').slideToggle('slow');
  });

  $('#startDate').datepicker( {dateFormat:'yy-mm-dd'} );
  $('#endDate').datepicker( {dateFormat:'yy-mm-dd'} );
  $('#doFilter').click(function(){
    $.ajax({ url: "dynamic",
             dataType: "xml",
             data: {action: "filter",
                    textSearch: $('#textSearch').val(),
                    startDate: $('#startDate').val(),
                    endDate: $('#endDate').val(),
                   },
             cache: false,
             type: "POST",
             success: function(data){
               if( $(data).find('error').text() ) {
                 alert("An error occured while updating the information.");
               } else {
                 $('#docList_table').fadeOut("fast");
                 filteredDocs = new Array();
                 $(data).find('docid').each( function() {
                   filteredDocs.push($(this).text());
                 });
                 master = 0;
                 applyMoveRow();
               }
             }
           });

  });
});

function applyMoveRow() {
  if(master >= allDocs.length) {
     $("#filterProgress").progressbar( "destroy" );
     $("#docList_table").trigger("rebuild");
     $('#docList_table').fadeIn();
  } else {
     $("#filterProgress").progressbar({ value: parseInt(100*(master/allDocs.length)) });
     if( $.inArray(allDocs[master], filteredDocs) > -1 ) {
       // we need to show this doc
       if ( $.inArray(allDocs[master], showingDocs) == -1 ) {
         showingDocs.push(allDocs[master]);
         // move from hidden to the lime lite
         tr = document.getElementById('docid_'+allDocs[master]);
//         tr = safeKeeping['docid_'+allDocs[master]];
         $("#docList_table").trigger("addRow",tr.cloneNode(true));
         document.getElementById('safeKeeping').getElementsByTagName('tbody')[0].removeChild(tr);
       }
     } else {
       // we need to hide this doc
       if ( $.inArray(allDocs[master], showingDocs) > -1 ) {
         showingDocs.splice($.inArray(allDocs[master], showingDocs), 1);
         // shuffle away for safe keeping
         $("#docList_table").trigger("removeRow",'docid_'+allDocs[master]);
       }
     }
     master++;
     setTimeout("applyMoveRow()", 1);
  }
}