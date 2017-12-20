create table rr_user_bank
(
	id		number constraint rr_user_bank_id primary key,
	name	varchar2(256),
	passwd	varchar2(9),
	balance number constraint rr_ueer_bank_ck check(balance > 0)
);

create sequence se_rr_user_bank_id
	minvalue 1
	maxvalue 9999999999
	increment by 1
	start with 1000
	nocycle;
