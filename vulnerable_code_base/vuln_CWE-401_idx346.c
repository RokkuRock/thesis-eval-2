destroyUserInformationLists(DUL_USERINFO * userInfo)
{
    PRV_SCUSCPROLE
    * role;
    role = (PRV_SCUSCPROLE*)LST_Dequeue(&userInfo->SCUSCPRoleList);
    while (role != NULL) {
        free(role);
        role = (PRV_SCUSCPROLE*)LST_Dequeue(&userInfo->SCUSCPRoleList);
    }
    LST_Destroy(&userInfo->SCUSCPRoleList);
    delete userInfo->extNegList; userInfo->extNegList = NULL;
    delete userInfo->usrIdent; userInfo->usrIdent = NULL;
}