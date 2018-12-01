`define SCREEN_WIDTH 176
`define SCREEN_HEIGHT 144

///////* DON'T CHANGE THIS PART *///////
module DE0_NANO(
	CLOCK_50,
	GPIO_0_D,
	GPIO_1_D,
	LED,
	KEY
);


//=======================================================
//  PARAMETER declarations
//=======================================================
localparam RED = 16'b11111_000000_00000;
localparam GREEN = 16'b00000_111111_00000;
localparam BLUE = 16'b00000_000000_11111;
localparam BLACK = 16'b00000_000000_00000;
//=======================================================
//  PORT declarations
//=======================================================

//////////// CLOCK - DON'T NEED TO CHANGE THIS //////////
input 		          		CLOCK_50;


////////////// LED ///////////////////////////////////////////
output 		    [7:0]		LED;


//////////// GPIO_0, GPIO_0 connect to GPIO Default //////////
output 		    [40:0]		GPIO_0_D;
//////////// GPIO_0, GPIO_1 connect to GPIO Default //////////
input 		    [40:0]		GPIO_1_D;
input 		     [1:0]		KEY;

///// PIXEL DATA /////
reg [15:0]	pixel_data_RGB565 = 16'd0;
reg [15:0]	pixel_data_EDGE   = 16'd0;
reg [31:0]	pixel_data_BOTH   = 32'd0;

///// READ/WRITE ADDRESS /////
reg [14:0] X_ADDR;
reg [14:0] Y_ADDR;
wire [14:0] WRITE_ADDRESS;
reg [14:0] READ_ADDRESS; 

assign WRITE_ADDRESS = X_ADDR + Y_ADDR*(`SCREEN_WIDTH);

///// VGA INPUTS/OUTPUTS /////
wire 			VGA_RESET;
wire [7:0]	VGA_COLOR_IN;
wire [9:0]	VGA_PIXEL_X;
wire [9:0]	VGA_PIXEL_Y;
wire [31:0]	MEM_OUTPUT;
wire			VGA_VSYNC_NEG;
wire			VGA_HSYNC_NEG;
reg			VGA_READ_MEM_EN;

assign GPIO_0_D[5] = VGA_VSYNC_NEG;
assign GPIO_0_D[0] = CLOCK_24_PLL;
assign VGA_RESET = ~KEY[0];

///// I/O for Img Proc /////
wire [2:0] RESULT;
wire [2:0] RESULT2;

/* WRITE ENABLE */
reg W_EN;

///////* CREATE ANY LOCAL WIRES YOU NEED FOR YOUR PLL *///////
wire 			CLOCK_24_PLL;
wire 			CLOCK_25_PLL;
wire  		CLOCK_50_PLL;

///////* INSTANTIATE YOUR PLL HERE *///////
team11PLL	team11PLL_inst (
	.inclk0 ( CLOCK_50 ),
	.c0 ( CLOCK_24_PLL ),
	.c1 ( CLOCK_25_PLL),
	.c2 ( CLOCK_50_PLL )
);

///////* M9K Module *///////
Dual_Port_RAM_M9K mem(
	.input_data(pixel_data_BOTH),
	.w_addr(WRITE_ADDRESS),
	.r_addr(READ_ADDRESS),
	.w_en(W_EN),
	.clk_W(CLOCK_50),
	.clk_R(CLOCK_25_PLL),
	.output_data(MEM_OUTPUT)
);

///////* VGA Module *///////
VGA_DRIVER driver (
	.RESET(VGA_RESET),
	.CLOCK(CLOCK_25_PLL),
	.PIXEL_COLOR_IN(VGA_READ_MEM_EN ? MEM_OUTPUT : {BLUE, 16'b0000000000000000}),
	.PIXEL_X(VGA_PIXEL_X),
	.PIXEL_Y(VGA_PIXEL_Y),
	.PIXEL_COLOR_OUT({GPIO_0_D[9],GPIO_0_D[11],GPIO_0_D[13],GPIO_0_D[15],GPIO_0_D[17],GPIO_0_D[19],GPIO_0_D[21],GPIO_0_D[23]}),
   .H_SYNC_NEG(GPIO_0_D[7]),
   .V_SYNC_NEG(VGA_VSYNC_NEG)
);

///////* Image Processor *///////
//IMAGE_PROCESSOR proc(
//	.PIXEL_IN(MEM_OUTPUT),
//	.CLK(CLOCK_25_PLL),
//	.VGA_PIXEL_X(VGA_PIXEL_X),
//	.VGA_PIXEL_Y(VGA_PIXEL_Y),
//	.VGA_VSYNC_NEG(VGA_VSYNC_NEG),
//	.VSYNC(VSYNC),
//	.HREF(HREF),
//	.RESULT(RESULT2)
//);


///////* Update Read Address *///////
always @ (VGA_PIXEL_X, VGA_PIXEL_Y) begin
		READ_ADDRESS = (VGA_PIXEL_X + VGA_PIXEL_Y*`SCREEN_WIDTH);
		if(VGA_PIXEL_X>(`SCREEN_WIDTH-1) || VGA_PIXEL_Y>(`SCREEN_HEIGHT-1))begin
				VGA_READ_MEM_EN = 1'b0;
		end
		else begin
				VGA_READ_MEM_EN = 1'b1;
		end
end


///////* Buffer Writer *///////
/*
integer i = 0;
integer j = 0;

always @(posedge CLOCK_24_PLL) begin
		W_EN = 1;
		
		X_ADDR = i;
		Y_ADDR = j;
		
		if (i < `SCREEN_WIDTH && (j % 10 == 0))
			pixel_data_RGB332 = RED;
		else
			pixel_data_RGB332 = BLACK;
			
		i = i+1;
			
		if (i == `SCREEN_WIDTH) begin
			i = 0;
			j = j+1;
			
			if (j == `SCREEN_HEIGHT) begin
				j = 0;
			end
		end
end
*/

///////* Downsampler *///////
reg[7:0] i = 0;
reg[7:0] j = 0;
reg      k = 0;

reg      h_flag = 0;
reg      v_flag = 0;

reg  [7:0]	input_1 = 8'd0;
reg  [7:0]	input_2 = 8'd0;
reg signed [7:0]	grey  = 8'd0;
reg signed [7:0]	grey_p = 8'd0;
reg signed [7:0]	grey_pp = 8'd0;
reg signed [7:0]	grey_ppp = 8'd0;
reg signed [7:0]	grey_pppp = 8'd0;
reg signed [7:0]	tmp_1 = 8'd0;
reg signed [7:0]	tmp_2 = 8'd0;
reg signed [7:0]	grey_eg = 8'd0;
reg signed [15:0]	grey_blur  = 16'd0;
wire [7:0]	CAMERA_INPUT;
assign 		CAMERA_INPUT = {GPIO_1_D[23],GPIO_1_D[21],GPIO_1_D[19],GPIO_1_D[17],GPIO_1_D[15],GPIO_1_D[13],GPIO_1_D[11],GPIO_1_D[9]};
wire			P_CLOCK;
assign		P_CLOCK = GPIO_1_D[7];
wire			HREF;
assign		HREF = GPIO_1_D[3];
wire			VSYNC;
assign		VSYNC = GPIO_1_D[5];

reg signed [5:0] redRDif;
reg signed [5:0] redGDif;
reg signed [5:0] redBDif;

reg signed [5:0] blueRDif;
reg signed [5:0] blueGDif;
reg signed [5:0] blueBDif;

reg [4:0] curBrightestVal;
reg signed [9:0] prevXB;
reg signed [9:0] curXB;
reg signed [10:0] xDiff;

reg [15:0] numStraight;
reg [15:0] numPos;
reg [15:0] numNeg;
reg [15:0] numRed;
reg [15:0] numBlue;

`define C_THRESH 17
`define B_THRESH 7

reg RES_2; 
reg RES_1;
reg RES_0;

assign  GPIO_0_D[33] = RES_2;
assign  GPIO_0_D[31] = RES_1;
assign  GPIO_0_D[29] = RES_0;

reg     LED_7;
reg     LED_6;
reg     LED_5;
reg     LED_4;
reg     LED_3;
reg     LED_2;
reg     LED_1;
reg     LED_0;

assign   LED[7] = LED_7;
assign   LED[6] = LED_6;
assign   LED[5] = LED_5;
assign   LED[4] = LED_4;
assign   LED[3] = LED_3;
assign   LED[2] = LED_2;
assign   LED[1] = LED_1;
assign   LED[0] = LED_0;

always @(posedge P_CLOCK) begin

	if (VSYNC == 1'b1 && v_flag == 0) begin
	RES_1 = 0;
	RES_0 = 0;
		if (numRed >= 6) begin
			RES_2 = 1;
			LED_3 = 0;
			LED_4 = 0;
			LED_5 = 0;
			if      ( numNeg >= 6 ) begin
				LED_0 = 1;
				RES_1 = 1;
	RES_0 = 0;
			end
			else begin
				LED_0 = 0;
			end
			if      ( numStraight >= 6 ) begin
			RES_1 = 0;
	RES_0 = 1;
				LED_1 = 1;
			end
			else begin
				LED_1 = 0;
			end
			if      ( numNeg >= 3 && numPos >= 3 ) begin
			RES_1 = 1;
	RES_0 = 1;
				LED_2 = 1;
			end
			else begin
				LED_2 = 0;
			end
		end
		else if (numBlue >= 6) begin
			RES_2 = 0;
			LED_0 = 0;
			LED_1 = 0;
			LED_2 = 0;
			if      ( numNeg >= 6 ) begin
				LED_3 = 1;
				RES_1 = 1;
	RES_0 = 0;
			end
			else begin
				LED_3 = 0;
			end
			if      ( numStraight >= 6 ) begin
				LED_4 = 1;
				RES_1 = 0;
	RES_0 = 1;
			end
			else begin
				LED_4 = 0;
			end
			if      ( numNeg >= 3 && numPos >= 3 ) begin
				LED_5 = 1;
				RES_1 = 1;
	RES_0 = 1;
			end
			else begin
				LED_5 = 0;
			end
		end
//		else if      ( numPos > 4 )      res = 3'b010; 
//		else if      ( numStraight > 4 )      res = 3'b100; 
//		else res = 3'b000; // sees nothing 
		i = 0;
		j = 0;
		v_flag = 1;
		numNeg = 0;
		numPos = 0;
		numRed = 0;
		numBlue = 0;
		numStraight = 0;
	end
	
	else if (VSYNC == 1'b0) begin
	
		v_flag = 0;
		
		if (HREF == 1'b0 && h_flag == 0) begin
			k = 0;
			i = 0;
			j = j+1;
			h_flag = 1;
		end
		
		else if (HREF == 1'b1) begin
			h_flag = 0;
			
			X_ADDR = i;
			Y_ADDR = j;
			
			if (k == 0) begin
				input_1 = CAMERA_INPUT;
				k = 1;
				W_EN = 0;
			end
			
			else begin
				input_2 = CAMERA_INPUT;
				
				pixel_data_RGB565 = {input_2, input_1};
				
				redRDif = 5'b11111 - pixel_data_RGB565[15:11];
				redGDif = 5'b01000 - pixel_data_RGB565[10:6];
				redBDif = 5'b01000 - pixel_data_RGB565[4:0];
				
				blueRDif = 5'b00001 - pixel_data_RGB565[15:11];
				blueGDif = 5'b00001 - pixel_data_RGB565[10:6];
				blueBDif = 5'b00101 - pixel_data_RGB565[4:0];
				
//				if (redRDif <  `C_THRESH && redRDif >  -`C_THRESH &&
//				    redGDif <  `C_THRESH && redGDif >  -`C_THRESH &&
//				    redBDif <  `C_THRESH && redBDif >  -`C_THRESH    )
//				  pixel_data_RGB565 = 16'b1111100000000000;
//				else if (blueRDif <  `B_THRESH && blueRDif >  -`B_THRESH &&
//				    blueGDif <  `B_THRESH && blueGDif >  -`B_THRESH &&
//				    blueBDif <  `B_THRESH && blueBDif >  -`B_THRESH    )
//				  pixel_data_RGB565 = 16'b0000000000011111;
//				else
//				  pixel_data_RGB565 = 16'b0000000000000000;
				  
				
				grey_pppp = grey_ppp;
				grey_ppp = grey_pp;
				grey_pp = grey_p;
				grey_p  = grey;
				grey = input_2[7:3] + {input_2[2:0], input_1[7:6]} + input_1[4:0];
				
				grey_blur = grey + grey_p;
				grey_blur = grey_blur >> 1;
				grey = grey_blur[7:0];
				
				/*
				tmp_1 = (grey_pppp + grey_ppp);
				tmp_2 = (grey_p + grey);
				grey_eg = tmp_2 - tmp_1;
				if (grey_eg[7]) begin
					grey_eg = 0;
				end
				grey_eg = grey_eg << 2;
				*/
				
				tmp_1 = grey - grey_pppp;
				if (tmp_1[7]) begin
					tmp_1 = !tmp_1 + 1;
				end
				tmp_2 = grey_p - grey_ppp;
				if (tmp_2[7]) begin
					tmp_2 = !tmp_2 + 1;
				end
				grey_eg = (tmp_1 + tmp_2) >> 1;
				grey_eg = grey_eg << 2;
				
				/*
				grey_eg = grey - grey_p;
				if (grey_eg[7]) begin
				  grey_eg = 0;
				end
				grey_eg = grey_eg << 2;
				*/
				//pixel_data_RGB565 = 16'b0000000000000000;
//				if (Y_ADDR > 20 && Y_ADDR < 130) begin
					if (Y_ADDR % 12 == 0 && X_ADDR >= 30 && X_ADDR < 150) begin
						if (grey_eg[7:3] > curBrightestVal) begin
							curBrightestVal = grey_eg[7:3];
							pixel_data_RGB565 = 16'b1111111111111111;
							curXB = X_ADDR;
						end
					end
					if (Y_ADDR % 12 == 0 && X_ADDR == 150 ) begin
						if (curBrightestVal >= 1 && curXB > 50) begin
							xDiff = curXB - prevXB;
							if (xDiff > 3 && xDiff < 16) begin
								pixel_data_RGB565 = 16'b1111100000000000;
								numNeg = numNeg + 1;
							end
							else if (xDiff < -3 && xDiff > -16) begin
								pixel_data_RGB565 = 16'b0000000000011111;
								numPos = numPos + 1;
							end
							else if (xDiff <= 3 && xDiff >= -3) begin
								pixel_data_RGB565 = 16'b0000011111100000;
								numStraight = numStraight + 1;
							end
							else begin
								pixel_data_RGB565 = 16'b1111111111111111;
							end
						end
						prevXB = curXB;
						curBrightestVal = 0;
					end
					if (Y_ADDR % 12 == 1 && X_ADDR == prevXB - 20) begin
						if (redRDif <  `C_THRESH && redRDif >  -`C_THRESH &&
								redGDif <  `C_THRESH && redGDif >  -`C_THRESH &&
								redBDif <  `C_THRESH && redBDif >  -`C_THRESH    )
							numRed = numRed + 1;
						else if (blueRDif <  `B_THRESH && blueRDif >  -`B_THRESH &&
								blueGDif <  `B_THRESH && blueGDif >  -`B_THRESH &&
								blueBDif <  `B_THRESH && blueBDif >  -`B_THRESH    )
							numBlue = numBlue + 1;
					end
//				end
				
//				if (X_ADDR == 150 ) begin
//					curBrightestVal = 0;
//				end
				
				pixel_data_EDGE = {grey_eg[7:3], grey_eg[7:2], grey_eg[7:3]};
				
				pixel_data_BOTH = {pixel_data_RGB565, pixel_data_EDGE};
				
				i = i+1;
				k = 0;
				W_EN = 1;
			end
		end
	end		
end

///////* Color Detection *///////
//

//
//always @(*) begin
//		/*
//		if (RESULT[0]) LED_0 = 1;
//      else LED_0 = 0;
//		
//		if (RESULT[1]) LED_1 = 1;
//      else LED_1 = 0;
//		
//		if (RESULT[2]) LED_2 = 1;
//      else LED_2 = 0;
//		*/
//		if (RESULT == 3'b001) begin
//			LED_0 = 0;
//			LED_1 = 1;
//			LED_2 = 0;
//			LED_3 = 0;
//		end else if (RESULT == 3'b010) begin
//			LED_0 = 0;
//			LED_1 = 0;
//			LED_2 = 1;
//			LED_3 = 0;
//		end else if (RESULT == 3'b100) begin
//			LED_0 = 0;
//			LED_1 = 0;
//		   LED_2 = 0;
//			LED_3 = 1;	
//		end else begin
//			LED_0 = 1;
//			LED_1 = 0;
//			LED_2 = 0;
//			LED_3 = 0;
//		end
//		
//end

	
endmodule 
