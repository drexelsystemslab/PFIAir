$('#morph_table > tbody > tr').filter(function( index ) {
    if((index + 1) % 4 == 0) return $(this);
}).each(function() {
	$(this).find('td').each(function() {
    	$(this).css('border-bottom', '1px black solid');
	});
});