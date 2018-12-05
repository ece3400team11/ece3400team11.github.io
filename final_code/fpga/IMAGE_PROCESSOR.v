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

	reg signed [9:0] prevXB;
	reg [4:0] curBrightestVal;
	reg signed [9:0] curXB;
	
	reg [15:0] numStraight;
	reg [15:0] numPos;
	reg [15:0] numNeg;
	
	reg signed [5:0] redRDif;
	reg signed [6:0] redGDif;
	reg signed [5:0] redBDif;
	
	reg signed [5:0] blueRDif;
	reg signed [6:0] blueGDif;
	reg signed [5:0] blueBDif;
	
reg   lastSync = 1'b0;

always @(posedge CLK) begin
	
	if (HREF) begin
		valR = PIXEL_IN[7:5];
	   
		if (VGA_PIXEL_Y % 4 == 0) begin
			if (PIXEL_IN[4:0] > curBrightestVal) begin
				curBrightestVal = PIXEL_IN[4:0];
				curXB = VGA_PIXEL_X;
			end
		end
		else if (VGA_PIXEL_Y % 4 == 1) begin
			if (VGA_PIXEL_X == curXB-1) begin
				redRDif = 5'b10000 - PIXEL_IN[31:27];
				redGDif = 5'b00000 - PIXEL_IN[26:21];
				redBDif = 5'b00000 - PIXEL_IN[20:16];
				
				blueRDif = 5'b00000 - PIXEL_IN[31:27];
				blueGDif = 5'b00000 - PIXEL_IN[26:21];
				blueBDif = 5'b10000 - PIXEL_IN[20:16];
				if (redRDif < 
			end	
		end
		
		if (VGA_PIXEL_Y > 20 && VGA_PIXEL_Y < 130) begin
			if (VGA_PIXEL_X == 150) begin
				if (curXB - prevXB > 4) begin
					numNeg++;
				end
				else if (curXB - prevXB < -4) begin
					numPos++;
				end
				else begin
					numStraight++;
				end

				prevXB = curXB;
				curBrightestVal = 0;
			end
		end
	end
	
	if (VSYNC == 1'b1 && lastSync == 1'b0) begin //posedge VSYNC
		if (numStraight > 10) res = 3'b111;
		else res = 3'b000;
	end
	
	if (VSYNC == 1'b0 && lastSync == 1'b1) begin //negedge VSYNC
		numB = 0;
		numR = 0;
		numN = 0;
		
		numNeg = 0;
		numPos = 0;
		numStraight = 0;
	end
	
	lastSync = VSYNC;

end

endmodule