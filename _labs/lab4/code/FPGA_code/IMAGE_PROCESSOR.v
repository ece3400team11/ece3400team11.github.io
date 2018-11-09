`define SCREEN_WIDTH 176
`define SCREEN_HEIGHT 144
`define NUM_BARS 3
`define BAR_HEIGHT 48

module IMAGE_PROCESSOR (
	PIXEL_IN,
	CLK,
	VGA_PIXEL_X,
	VGA_PIXEL_Y,
	VGA_VSYNC_NEG,
	VSYNC,
	HREF,
	RESULT
);


//=======================================================
//  PORT declarations
//=======================================================
input	[7:0]	PIXEL_IN;
input 		CLK;

input [9:0] VGA_PIXEL_X;
input [9:0] VGA_PIXEL_Y;
input			VGA_VSYNC_NEG;
input			VSYNC;
input			HREF;


output [8:0] RESULT;
reg          res;
assign       RESULT = res;

reg   [2:0]  valR;
reg   [2:0]  valG;
reg   [2:0]  valB;
 
reg   [15:0] numB;
reg   [15:0] numR;
reg   [15:0] numN;
reg   [15:0] threshB = 16'd20000;
reg   [15:0] threshR = 16'd20000;


reg   lastSync = 1'b0;

always @(posedge CLK) begin
	
	if (HREF) begin
	   valR = PIXEL_IN[7:5];
	   valG = PIXEL_IN[4:2];
	   valB = {PIXEL_IN[1:0], 1'b0};
		
		if (valB > valR && valB > valG) numB = numB + 16'd1;
		else if (valR > valB && valR > valG) numR = numR + 16'd1;
		else numN = numN + 16'd1;
	end
	
	if (VSYNC == 1'b1 && lastSync == 1'b0) begin //posedge VSYNC
		if (numB > threshB) res = 3'b111;
		else if (numR > threshR) res = 3'b110;
		else res = 3'b000;
	end
	
	if (VSYNC == 1'b0 && lastSync == 1'b1) begin //negedge VSYNC
		numB = 0;
		numR = 0;
		numN = 0;
	end
	
	lastSync = VSYNC;

end

endmodule